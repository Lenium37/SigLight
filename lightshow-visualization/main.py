from tkinter import Tk
import tkinter as tk
from fixture import Fixture
import xml.etree.ElementTree as ET
import wave
import threading
import soundfile
from utils import *


# load xml file
xml_tree = ET.parse('C:\\Users\\Johannes\\Music\\Raspberry-to-Light\\xml-lightshows\\Fun.-Some-Nights.xml')
xml_root = xml_tree.getroot()

# fill arrays with colors
if xml_root.attrib["res"] == "40":
    vc_bass = [(255, 0, 0, 0)] * int(xml_root.attrib["length"])
    vc_mid = [(0, 0, 255, 0)] * int(xml_root.attrib["length"])
    vc_high = [(0, 255, 0, 0)] * int(xml_root.attrib["length"])
    vc_ambient = [(255, 255, 0, 0)] * int(xml_root.attrib["length"])
else:
    print("ERROR: Unknown resolution of lightshow!")

# load sound file
sound_file = xml_root.attrib["src"]
# print name
print("Playing", sound_file)
# convert to 16 bit WAV
data, samplerate = soundfile.read(sound_file)
soundfile.write('converted.wav', data, samplerate, subtype='PCM_16')
# open converted sound file
wf = wave.open("converted.wav", 'rb')

# read in Lightshow and fill arrays
for fixture in range(0, len(xml_root.findall('fixture'))):
    number_of_channels = len(xml_root[fixture].findall('channel'))
    fixture_type = xml_root[fixture].attrib["type"]
    #print(fixture_type)
    #print("Number of channels this fix has: " + number_of_channels.__str__())
    for channel in range(0, number_of_channels):
        channel_number = int(xml_root[fixture][channel].attrib["channel"])
        number_of_value_changes = len(xml_root[fixture][channel].findall('vc'))
        #print("Number of value changes channel " + channel_number.__str__() + " has: " + number_of_value_changes.__str__())
        for vc in range(0, number_of_value_changes):
            timestamp_in_array = int(round_to_lightshow_frame(float(xml_root[fixture][channel][vc].attrib["t"])) * 40)
            if fixture_type == "bass":
                if channel_number == 1:
                    list_temp = list(vc_bass[timestamp_in_array])
                    list_temp[3] = int(xml_root[fixture][channel][vc].text)
                    vc_bass[timestamp_in_array] = tuple(list_temp)
                #elif channel_number == 3:
                #    list_temp = list(vc_bass[timestamp_in_array])
                #    list_temp[0] = int(xml_root[fixture][channel][vc].text)
                #    vc_bass[timestamp_in_array] = tuple(list_temp)
                #elif channel_number == 4:
                #    list_temp = list(vc_bass[timestamp_in_array])
                #    list_temp[1] = int(xml_root[fixture][channel][vc].text)
                #    vc_bass[timestamp_in_array] = tuple(list_temp)
                #elif channel_number == 5:
                #    list_temp = list(vc_bass[timestamp_in_array])
                #    list_temp[2] = int(xml_root[fixture][channel][vc].text)
                #    vc_bass[timestamp_in_array] = tuple(list_temp)

            elif fixture_type == "mid":
                if channel_number == 1:
                    list_temp = list(vc_mid[timestamp_in_array])
                    list_temp[3] = int(xml_root[fixture][channel][vc].text)
                    vc_mid[timestamp_in_array] = tuple(list_temp)
                #elif channel_number == 3:
                #    list_temp = list(vc_mid[timestamp_in_array])
                #    list_temp[0] = int(xml_root[fixture][channel][vc].text)
                #    vc_mid[timestamp_in_array] = tuple(list_temp)
                #elif channel_number == 4:
                #    list_temp = list(vc_mid[timestamp_in_array])
                #    list_temp[1] = int(xml_root[fixture][channel][vc].text)
                #    vc_mid[timestamp_in_array] = tuple(list_temp)
                #elif channel_number == 5:
                #    list_temp = list(vc_mid[timestamp_in_array])
                #    list_temp[2] = int(xml_root[fixture][channel][vc].text)
                #    vc_mid[timestamp_in_array] = tuple(list_temp)

            elif fixture_type == "high":
                if channel_number == 1:
                    list_temp = list(vc_high[timestamp_in_array])
                    list_temp[3] = int(xml_root[fixture][channel][vc].text)
                    vc_high[timestamp_in_array] = tuple(list_temp)
                #elif channel_number == 3:
                #    list_temp = list(vc_high[timestamp_in_array])
                #    list_temp[0] = int(xml_root[fixture][channel][vc].text)
                #    vc_high[timestamp_in_array] = tuple(list_temp)
                #elif channel_number == 4:
                #    list_temp = list(vc_high[timestamp_in_array])
                #    list_temp[1] = int(xml_root[fixture][channel][vc].text)
                #    vc_high[timestamp_in_array] = tuple(list_temp)
                #elif channel_number == 5:
                #    list_temp = list(vc_high[timestamp_in_array])
                #    list_temp[2] = int(xml_root[fixture][channel][vc].text)
                #    vc_high[timestamp_in_array] = tuple(list_temp)

            elif fixture_type == "ambient":
                if channel_number == 1:
                    list_temp = list(vc_ambient[timestamp_in_array])
                    list_temp[3] = int(xml_root[fixture][channel][vc].text)
                    vc_ambient[timestamp_in_array] = tuple(list_temp)
                #elif channel_number == 3:
                #    list_temp = list(vc_ambient[timestamp_in_array])
                #    list_temp[0] = int(xml_root[fixture][channel][vc].text)
                #    vc_ambient[timestamp_in_array] = tuple(list_temp)
                #elif channel_number == 4:
                #    list_temp = list(vc_ambient[timestamp_in_array])
                #    list_temp[1] = int(xml_root[fixture][channel][vc].text)
                #    vc_ambient[timestamp_in_array] = tuple(list_temp)
                #elif channel_number == 5:
                #    list_temp = list(vc_ambient[timestamp_in_array])
                #    list_temp[2] = int(xml_root[fixture][channel][vc].text)
                #    vc_ambient[timestamp_in_array] = tuple(list_temp)

# fill all the blank (black) frames with the previous used color
#for array in (vc_bass, vc_mid, vc_high, vc_ambient):
#    for i in range(0, len(array)):
#        list_temp = list(array[i])
#        if list_temp[0] == 0 and list_temp[1] == 0 and list_temp[2] == 0:
#            if i > 0:
#                prev_list_temp = list(array[i - 1])
#                list_temp[0] = prev_list_temp[0]
#                list_temp[1] = prev_list_temp[1]
#                list_temp[2] = prev_list_temp[2]
#                array[i] = tuple(list_temp)

# ambient alpha channel should always stay on
for i in range(0, len(vc_ambient)):
    list_temp = list(vc_ambient[i])
    if list_temp[3] == 0:
        if i > 0:
            prev_list_temp = list(vc_ambient[i - 1])
            list_temp[3] = prev_list_temp[3]
            vc_ambient[i] = tuple(list_temp)


# print arrays
print("Bass:", vc_bass)
print("Mid:", vc_mid)
print("High:", vc_high)
print("Ambient:", vc_ambient)

# create canvas
root = Tk()
root.wm_title("Lightshow Visualization")
root.resizable(0, 0)
root.wm_attributes("-topmost", 1)
canvas = tk.Canvas(root, width=1800, height=900, borderwidth=0, highlightthickness=0)

# create Fixtures
fix_bass = Fixture(canvas, (255, 0, 0, 0), 0, vc_bass)
fix_mid = Fixture(canvas, (0, 0, 255, 0), 1, vc_mid)
fix_high = Fixture(canvas, (0, 255, 0, 0), 2, vc_high)
fix_ambient = Fixture(canvas, (0, 255, 255, 0), 3, vc_ambient)

# initiate Fixtures
fix_bass.update_color()
fix_mid.update_color()
fix_high.update_color()
fix_ambient.update_color()

# play Audio
player = Player(wf)
t = threading.Thread(target=player.play_audio)
t.daemon = True
t.start()

root.mainloop()
