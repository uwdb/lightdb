#!/usr/bin/env python

import sys
import cv2

filename = sys.argv[1]
target_rgb = map(int, sys.argv[2].split(','))
threshold = int(sys.argv[3])

input = cv2.VideoCapture(filename)
index = 0

while input.isOpened():
    ret, frame = input.read()
    index += 1

    if frame is not None:
        for bgr in frame[0]:
            rgb = bgr[::-1]
            for color, left_value, right_value in zip(('red', 'green', 'blue'), rgb, target_rgb):
                if abs(left_value - right_value) > threshold:
                    print 'Frame {} had {} that exceeded threshold ({} vs {})'.format(index, color, left_value, right_value)
                    exit(1)
    else:
        exit(0)