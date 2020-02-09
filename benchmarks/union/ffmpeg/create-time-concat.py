import sys

concat = "-i 'concat:segment1.h264"
for i in xrange(1, int(sys.argv[1]) + 1):
	concat += "|segment{}.h264".format(i)
concat += "'"

print "ffmpeg -y -hide_banner -loglevel error {} -c copy out.mp4".format(concat)
