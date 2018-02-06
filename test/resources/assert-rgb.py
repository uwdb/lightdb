#!/usr/bin/env python

import sys
import cv2

left_filename = sys.argv[1]
right_filename = sys.argv[2]
threshold = sys.argv[3]

left = cv2.VideoCapture(left_filename)
right = cv2.VideoCapture(right_filename)
index = 0

while left.isOpened() and right.isOpened():
    left_ret, left_frame = left.read()
    right_ret, right_frame = right.read()
    index += 1

    if left_frame is not None and right_frame is not None:
        left_rgb = left_frame.mean(axis=(0, 1))
        right_rgb = right_frame.mean(axis=(0, 1))

        for color, left_value, right_value in zip(('red', 'green', 'blue'), left_rgb, right_rgb):
            if abs(left_value - right_value) > threshold:
                print 'Frame {} had {} that exceeded threshold ({} vs {})'.format(index, color, left_value, right_value)
                exit(1)
    elif left_frame is None and right_frame is not None:
        print '{} has frame {} with no corresponding frame in {}'.format(left_filename, index, right_filename)
        exit(1)
    elif right_frame is None and left_frame is not None:
        print '{} has frame {} with no corresponding frame in {}'.format(right_filename, index, left_filename)
        exit(1)
    else:
        exit(0)


