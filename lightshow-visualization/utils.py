import pyaudio
import time


def round_to_lightshow_frame(x, base=0.025):
    return round(base * round(x/base), 3)


class Player:
    def __init__(self, wf):
        self.wav_file = wf

    # define callback (2)
    def callback(self, in_data, frame_count, time_info, status):
        data = self.wav_file.readframes(frame_count)
        return (data, pyaudio.paContinue)

    def play_audio(self):
        # instantiate PyAudio (1)
        p = pyaudio.PyAudio()

        # open stream using callback (3)
        stream = p.open(format=p.get_format_from_width(self.wav_file.getsampwidth()),
                        channels=self.wav_file.getnchannels(),
                        rate=self.wav_file.getframerate(),
                        output=True,
                        stream_callback=self.callback)

        # start the stream (4)
        stream.start_stream()

        # wait for stream to finish (5)
        while stream.is_active():
            time.sleep(0.1)

        # stop stream (6)
        stream.stop_stream()
        stream.close()
        self.wav_file.close()

        # close PyAudio (7)
        p.terminate()
