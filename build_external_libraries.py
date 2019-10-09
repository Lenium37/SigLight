import contextlib
import glob
import logging
import multiprocessing
import os
import shutil
import subprocess
import sys
import tarfile
import tempfile
import time

import bz2

try:
    from urllib2 import urlopen
except ImportError:
    from urllib.request import urlopen

from collections import OrderedDict

ROOT = os.path.dirname(__file__)

logger = logging.getLogger("raspitolight")
logger.setLevel(logging.INFO)
logger.addHandler(logging.StreamHandler())

time_results = OrderedDict()


@contextlib.contextmanager
def change_dir(new_dir):
    old_dir = os.getcwd()
    os.chdir(new_dir)

    yield

    os.chdir(old_dir)


class Step(object):
    _name = None
    _library_name = None

    def __init__(self, library_name):
        self._library_name = library_name

    def execute(self):
        global time_results

        start = time.time()
        if self._has_to_execute():
            logger.info("{}: executing step '{}'".format(self._library_name, self._name))
            self._do_execute()
            logger.info("{}: marking '{}' as done".format(self._library_name, self._name))
            self._mark_as_executed()
        else:
            logger.info("{}: step '{}' is up-to-date".format(self._library_name, self._name))

        execution_time = time.time() - start
        time_results['{} {}'.format(self._library_name, self._name)] = execution_time
        logger.info("{}: step '{} took {} seconds".format(self._library_name, self._name, execution_time))

    def _do_execute(self):
        raise NotImplementedError()

    def hash(self):
        raise NotImplementedError()

    def _has_to_execute(self):
        return not os.path.exists(os.path.join(self._get_operating_folder(), self._get_marker_name()))

    def _mark_as_executed(self):
        with open(os.path.join(self._get_operating_folder(), self._get_marker_name()), "w") as f:
            f.write("executed")

    def _get_operating_folder(self):
        raise NotImplementedError()

    def _get_marker_name(self):
        return "{}_{}_{}".format(self._library_name, self._name, self.hash())


class GitCloneStep(Step):
    _name = "Git clone"

    def __init__(self, library_name, url):
        super(GitCloneStep, self).__init__(library_name)
        self._url = url

    def hash(self):
        return hash(self._url)

    def _do_execute(self):
        with change_dir(self._get_operating_folder()):
            if os.path.exists(self._library_name):
                logger.info("Removing old folder")
                os.remove(self._library_name)
            cmd = ["git", "clone", "{}".format(self._url)]
            logger.info(cmd)
            subprocess.call(cmd)

    def _get_operating_folder(self):
        return os.path.join('external_libraries', self._library_name, 'source')


class CallCmakeStep(Step):
    _name = "CMake"

    def __init__(self, library_name, cmake_args=None):
        super(CallCmakeStep, self).__init__(library_name)
        self._cmake_args = cmake_args

    def _do_execute(self):
        if os.path.exists(self._get_operating_folder()):
            logger.info("Removing folder {}".format(self._get_operating_folder()))
            os.remove(self._get_operating_folder())
        else:
            os.makedirs(self._get_operating_folder())
        with change_dir(self._get_operating_folder()):
            cmd = ['cmake', '-G', '{}'.format(self._get_generator()), '-DCMAKE_INSTALL_PREFIX=..\install']
            if self._cmake_args:
                cmd.extend(self._cmake_args)
            cmd.append('..\source\{}'.format(self._library_name))
            logger.info(cmd)
            subprocess.call(cmd)

    def hash(self):
        return hash(self._library_name)

    def _get_operating_folder(self):
        return os.path.join(ROOT, 'external_libraries', self._library_name, 'build')

    def _get_generator(self):
        if sys.platform == 'win32':
            return "MinGW Makefiles"
        else:
            return "Unix Makefiles"


class CallMake(Step):
    _name = 'make'

    def __init__(self, library_name, make_args=None):
        super(CallMake, self).__init__(library_name)
        self._make_args = make_args

    def _do_execute(self):
        with change_dir(self._get_operating_folder()):
            cmd = ['{}'.format(self._get_make_name()), 'install', '-j', '{}'.format(multiprocessing.cpu_count())]
            logger.info(cmd)
            subprocess.call(cmd)

    def hash(self):
        return hash((self._library_name, self._make_args))

    def _get_operating_folder(self):
        return os.path.join(ROOT, 'external_libraries', self._library_name, 'build')

    def _get_make_name(self):
        if sys.platform == 'win32':
            return 'mingw32-make'
        else:
            return 'make'


class InstallLibUsbUnix(Step):
    _name = 'install_lib_usb_unix'

    def __init__(self, library_name):
        super(InstallLibUsbUnix, self).__init__(library_name)

    def _do_execute(self):
        install_folder = os.path.join(ROOT, 'external_libraries', self._library_name, 'install')
        install_folder = os.path.abspath(install_folder)

        src = os.path.join(ROOT, 'external_libraries', self._library_name, 'install', 'include', 'libusb-1.0',
                           'libusb.h')
        src = os.path.abspath(src)
        target = os.path.join(ROOT, 'external_libraries', self._library_name, 'install', 'include', 'libusb.h')
        target = os.path.abspath(target)

        with change_dir(self._get_operating_folder()):
            cmd = ['./autogen.sh']
            logger.info(cmd)
            subprocess.call(cmd)
            cmd = ['./configure', '--prefix={}'.format(install_folder)]
            logger.info(cmd)
            subprocess.call(cmd)
            cmd = ['make', 'install']
            logger.info(cmd)
            subprocess.call(cmd)
            shutil.copy(src, target)

    def hash(self):
        return hash((self._library_name))

    def _get_operating_folder(self):
        return os.path.join(ROOT, 'external_libraries', self._library_name, 'source', 'libusb')


class Library(object):

    def __init__(self, library_name, steps):
        self._library_name = library_name
        self._steps = steps

    def download_and_install(self):
        self._create_library_dirs()

        for step in self._steps:
            step.execute()

    def _create_library_dirs(self):
        dirs = ['external_libraries/{library_name}/source',
                'external_libraries/{library_name}/install']

        for d in dirs:
            d = d.format(library_name=self._library_name)
            if not os.path.exists(d):
                os.makedirs(d)

    @property
    def source_folder(self):
        return os.path.join(ROOT, 'external_libraries', self._library_name, 'source')

    @property
    def install_folder(self):
        return os.path.join(ROOT, 'external_libraries', self._library_name, 'install')

    @property
    def build_folder(self):
        return os.path.join(ROOT, 'external_libraries', self._library_name, 'build')


class DownloadArchive(Step):
    _name = "download archive"

    def __init__(self, library_name, url, archive_name):
        super(DownloadArchive, self).__init__(library_name)
        self._url = url
        self._archive_name = archive_name

    def _do_execute(self):
        # todo delete existing file
        filedata = urlopen(self._url)
        datatowrite = filedata.read()

        with open(os.path.join(self._get_operating_folder(), self._archive_name),
                  'wb') as f:
            f.write(datatowrite)

    def hash(self):
        return hash((self._library_name, self._url, self._archive_name))

    def _get_operating_folder(self):
        return os.path.join(ROOT, 'external_libraries', self._library_name, 'source')


class ExtractSourceTarArchive(Step):
    _name = "extract source archive"

    def __init__(self, library_name, archive_name):
        super(ExtractSourceTarArchive, self).__init__(library_name)
        self._archive_name = archive_name

    def _do_execute(self):
        # todo delete existing folder
        # todo cross platform using python tarfile module
        if sys.platform == 'win32':
            target_folder = os.path.join(self._get_operating_folder(), self._library_name + "_extract")
            cmd = r'"C:\Program Files\7-Zip\7z.exe" e {source_archive} -o{target_folder}'.format(
                source_archive=os.path.join(os.path.join(self._get_operating_folder(), self._archive_name)),
                target_folder=target_folder)
            logger.info(cmd)
            subprocess.call(cmd)

            cmd = r'"C:\Program Files\7-Zip\7z.exe" x {source_archive} -o{target_folder}'.format(
                source_archive=os.path.join(target_folder, self._archive_name.replace(".gz", "")),
                target_folder=self._get_operating_folder())
            logger.info(cmd)
            subprocess.call(cmd)

            logger.info(
                "{} => {}".format(os.path.join(self._get_operating_folder(), self._archive_name.replace(".tar.gz", "")),
                                  os.path.join(os.path.basename(target_folder), self._library_name)))
            os.rename(os.path.join(self._get_operating_folder(), self._archive_name.replace(".tar.gz", "")),
                      os.path.join(os.path.dirname(target_folder), self._library_name))
        else:
            cmd = ['tar', 'zxvf', '{}'.format(os.path.join(self._get_operating_folder(), self._archive_name)), '-C',
                   '{}'.format(self._get_operating_folder())]
            logger.info(cmd)
            subprocess.call(cmd)
            os.rename(os.path.join(self._get_operating_folder(), self._archive_name.replace(".tar.gz", "")),
                      os.path.join(self._get_operating_folder(), self._library_name))

    def hash(self):
        return hash((self._library_name, self._archive_name))

    def _get_operating_folder(self):
        # todo we can make this more beautiful
        return os.path.join(ROOT, 'external_libraries', self._library_name, 'source')


class ExtractSource7zArchive(Step):
    _name = "extract source archive"

    def __init__(self, library_name, archive_name):
        super(ExtractSource7zArchive, self).__init__(library_name)
        self._archive_name = archive_name

    def _do_execute(self):
        # todo delete existing folder
        # todo cross platform using python tarfile module
        if sys.platform == 'win32':
            target_folder = os.path.join(self._get_operating_folder(), self._library_name + "_extract")
            cmd = r'"C:\Program Files\7-Zip\7z.exe" x {source_archive} -o{target_folder} -aoa'.format(
                source_archive=os.path.join(os.path.join(self._get_operating_folder(), self._archive_name)),
                target_folder=target_folder)
            logger.info(cmd)
            subprocess.call(cmd)

        else:
            cmd = ['tar', 'zxvf', '{}'.format(os.path.join(self._get_operating_folder(), self._archive_name)), '-C',
                   '{}'.format(self._get_operating_folder())]
            logger.info(cmd)
            subprocess.call(cmd)
            os.rename(os.path.join(self._get_operating_folder(), self._archive_name.replace(".tar.gz", "")),
                      os.path.join(self._get_operating_folder(), self._library_name))

    def hash(self):
        return hash((self._library_name, self._archive_name))

    def _get_operating_folder(self):
        # todo we can make this more beautiful
        return os.path.join(ROOT, 'external_libraries', self._library_name, 'source')


def check_prerequsities():
    if sys.platform == 'win32':
        if not os.path.exists(r'C:\Program Files\7-Zip\7z.exe'):
            raise RuntimeError("7zip was not found! PLease download it at https://www.7-zip.de/")

        try:
            subprocess.call("mingw32-make --version")
        except OSError:
            raise RuntimeError(
                "mingw32-make is not in your $PATH. Please check https://projectbase.medien.hs-duesseldorf.de/cantes/19ss-raspitolight1/wikis/3.3.%20Einrichten%20des%20Projekts%20auf%20dem%20eigenen%20PC")
    try:
        subprocess.call(["git", "--version"])
    except OSError:
        raise RuntimeError("git is not in your $PATH or not installed!")
    try:
        subprocess.call(["cmake", "--version"])
    except OSError:
        raise RuntimeError(
            "CMake is not in your $PATH or not installed! You need at least CMake 3.13. Get it here: https://cmake.org/download/")


class CopyLibUsb(Step):
    _name = "copy lib usb"

    def __init__(self, library_name):
        super(CopyLibUsb, self).__init__(library_name)

    def _do_execute(self):
        include_folder = os.path.join(ROOT, 'external_libraries', self._library_name, 'install', 'include')
        lib_folder = os.path.join(ROOT, 'external_libraries', self._library_name, 'install', 'lib')

        if not os.path.exists(include_folder):
            os.makedirs(include_folder)

        if not os.path.exists(lib_folder):
            os.makedirs(lib_folder)

        shutil.copy(os.path.join(ROOT, 'external_libraries', self._library_name, 'source', 'libusb_extract', 'MinGW64',
                                 'static', 'libusb-1.0.a'),
                    os.path.join(ROOT, 'external_libraries', self._library_name, 'install', 'lib', 'libusb-1.0.a'))

        shutil.copy(os.path.join(ROOT, 'external_libraries', self._library_name, 'source', 'libusb_extract', 'include',
                                 'libusb-1.0', 'libusb.h'),
                    os.path.join(ROOT, 'external_libraries', self._library_name, 'install', 'include', 'libusb.h'))

    def hash(self):
        return hash('CopyLibUsb')

    def _get_operating_folder(self):
        return os.path.join(ROOT, 'external_libraries', self._library_name, 'source')


class CopyMingwThread(Step):
    _name = "copy mingw thread"

    def __init__(self, library_name):
        super(CopyMingwThread, self).__init__(library_name)

    def _do_execute(self):
        include_folder = os.path.join(ROOT, 'external_libraries', self._library_name, 'install', 'include')

        if not os.path.exists(include_folder):
            os.makedirs(include_folder)

        for f in glob.glob(
                os.path.join(ROOT, 'external_libraries', self._library_name, 'source', 'mingw-std-threads', '*.h')):
            shutil.copy(f, os.path.join(ROOT, 'external_libraries', self._library_name, 'install', 'include',
                                        os.path.basename(f)))

    def hash(self):
        return hash('CopyLibUsb')

    def _get_operating_folder(self):
        return os.path.join(ROOT, 'external_libraries', self._library_name, 'source')


class ExtractTarBz2(Step):
    _name = "ExtractTarBz2"

    def __init__(self, library_name, archive_name):
        super(ExtractTarBz2, self).__init__(library_name)
        self._archive_name = archive_name

    def hash(self):
        return hash('ExtractTarBz2')

    def _do_execute(self):
        filepath = os.path.join(self._get_operating_folder(), self._archive_name)

        zipfile = bz2.BZ2File(filepath)  # open the file
        data = zipfile.read()  # get the decompressed data
        newfilepath = filepath[:-4]  # assuming the filepath ends with .bz2
        open(newfilepath, 'wb').write(data)  # write a uncompressed file

        tar = tarfile.open(filepath[:-4])
        with change_dir(self._get_operating_folder()):
            tar.extractall()
            tar.close()

    def _get_operating_folder(self):
        return os.path.join(ROOT, 'external_libraries', self._library_name, 'source')


class ExtractTarGz(Step):
    _name = "ExtractTarBz2"

    def __init__(self, library_name, archive_name):
        super(ExtractTarGz, self).__init__(library_name)
        self._archive_name = archive_name

    def hash(self):
        return hash('ExtractTarGz')

    def _do_execute(self):
        filepath = os.path.join(self._get_operating_folder(), self._archive_name)

        tar = tarfile.open(filepath, "r:gz")
        with change_dir(self._get_operating_folder()):
            tar.extractall()
            tar.close()

    def _get_operating_folder(self):
        return os.path.join(ROOT, 'external_libraries', self._library_name, 'source')


class FixlibftdiFolderName(Step):
    _name = "Renamelibftdi"

    def __init__(self, library_name):
        super(FixlibftdiFolderName, self).__init__(library_name)

    def hash(self):
        return hash('Renamelibftdi')

    def _do_execute(self):
        current_name = os.path.join(self._get_operating_folder(), 'libftdi1-1.4')
        target_name = os.path.join(self._get_operating_folder(), 'libftdi')

        os.rename(current_name, target_name)

    def _get_operating_folder(self):
        return os.path.join(ROOT, 'external_libraries', self._library_name, 'source')


def command_completes(command):
    try:
        subprocess.call(command)
        return True
    except:
        return False


def download_pip():
    filedata = urlopen('https://bootstrap.pypa.io/get-pip.py')
    datatowrite = filedata.read()

    get_pip_path = os.path.join(tempfile.gettempdir(), 'get-pip.py')
    with open(get_pip_path, 'wb') as f:
        f.write(datatowrite)

    subprocess.call([sys.executable, get_pip_path])


def download_conan():
    subprocess.call(['pip', 'install', 'conan'])


class ApplyLameCmakeInstallFix(Step):
    FIX_STR = """install(TARGETS ${PROJECT_NAME}
LIBRARY DESTINATION ${CMAKE_INSTALL_PREFIX}/bin
PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_PREFIX}/include
)"""


    def _do_execute(self):
        cmake_lists_txt = os.path.join(ROOT, 'external_libraries', self._library_name, 'source', self._library_name,
                                       'CMakeLists.txt')

        with open(cmake_lists_txt, 'a') as f:
            f.write("\n\n")
            f.write(self.FIX_STR)

    def hash(self):
        return hash('lame')

    def _get_operating_folder(self):
        return os.path.join(ROOT, 'external_libraries', self._library_name, 'source')


if __name__ == '__main__':

    check_prerequsities()

    libsndfile = Library(library_name='libsndfile',
                         steps=[
                             GitCloneStep('libsndfile', 'https://github.com/erikd/libsndfile.git'),
                             CallCmakeStep('libsndfile'),
                             CallMake('libsndfile')
                         ])

    libsndfile.download_and_install()

    fftw3 = Library(library_name='fftw3',
                    steps=[
                        DownloadArchive('fftw3', 'http://www.fftw.org/fftw-3.3.8.tar.gz', 'fftw-3.3.8.tar.gz'),
                        ExtractSourceTarArchive('fftw3', 'fftw-3.3.8.tar.gz'),
                        CallCmakeStep('fftw3', ['-DBUILD_SHARED_LIBS=OFF']),
                        CallMake('fftw3')
                    ])

    fftw3.download_and_install()

    if sys.platform == 'win32':
        mingw_thread = Library(library_name='mingw_thread',
                               steps=[
                                   GitCloneStep('mingw_thread', 'https://github.com/meganz/mingw-std-threads.git'),
                                   CopyMingwThread('mingw_thread')
                               ])

        mingw_thread.download_and_install()

        libusb = Library(library_name='libusb',
                         steps=[
                             DownloadArchive('libusb',
                                             'https://netix.dl.sourceforge.net/project/libusb/libusb-1.0/libusb-1.0.22/libusb-1.0.22.7z',
                                             'libusb-1.0.22.7z'),
                             ExtractSource7zArchive('libusb', 'libusb-1.0.22.7z'),
                             CopyLibUsb('libusb')
                         ])

        libusb.download_and_install()
    else:
        libusb = Library(library_name='libusb',
                         steps=[
                             GitCloneStep('libusb', 'https://github.com/libusb/libusb.git'),
                             InstallLibUsbUnix('libusb')
                         ])

        libusb.download_and_install()

    portaudio = Library(library_name='portaudio',
                        steps=[
                            GitCloneStep('portaudio', 'https://git.assembla.com/portaudio.git'),
                            CallCmakeStep('portaudio', ['-DPA_BUILD_SHARED=OFF']),
                            CallMake('portaudio')
                        ])
    portaudio.download_and_install()

    if not command_completes(["pip", "--version"]):
        download_pip()
    else:
        logger.info("pip already installed")

    if not command_completes(["conan", "--version"]):
        download_conan()
        logger.info("conan already installed")

    libftdi = Library(library_name='libftdi',
                      steps=[
                          DownloadArchive('libftdi',
                                          'https://www.intra2net.com/en/developer/libftdi/download/libftdi1-1.4.tar.bz2',
                                          'libftdi1-1.4.tar.bz2'),
                          ExtractTarBz2('libftdi', 'libftdi1-1.4.tar.bz2'),
                          FixlibftdiFolderName('libftdi'),
                          CallCmakeStep('libftdi', [
                              r'-DPC_LIBUSB_INCLUDEDIR={}'.format(os.path.join(libusb.install_folder, 'include')),
                              r'-DPC_LIBUSB_LIBDIR={}'.format(os.path.join(libusb.install_folder, 'lib'))]),
                          CallMake('libftdi')
                      ])

    # NOTE: disabled for now, because we dont need it right now
    # libftdi.download_and_install()

    taglib = Library(library_name='taglib-1.11.1',
                     steps=[
                         DownloadArchive('taglib-1.11.1',
                                         'https://taglib.org/releases/taglib-1.11.1.tar.gz',
                                         'taglib.tar.gz'),
                         ExtractTarGz('taglib-1.11.1',
                                      'taglib.tar.gz'),
                         CallCmakeStep('taglib-1.11.1'),
                         CallMake('taglib-1.11.1')
                     ])

    taglib.download_and_install()

    lame = Library(library_name='libmp3lame-CMAKE',
                   steps=[
                       GitCloneStep('libmp3lame-CMAKE', 'https://github.com/R-Tur/libmp3lame-CMAKE.git'),
                       ApplyLameCmakeInstallFix('libmp3lame-CMAKE'),
                       CallCmakeStep('libmp3lame-CMAKE'),
                       CallMake('libmp3lame-CMAKE'),
                   ])
    lame.download_and_install()

    logger.info("Done")
    logger.info("\n")

    for key, value in time_results.items():
        logger.info('Step "{}" took {} seconds to execute'.format(key, value))
