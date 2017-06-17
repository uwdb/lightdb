from dashtranscoder import DashTranscoder

print 'Tiler: qqq'
print len(DashTranscoder().tile('foo', 3, 4).tobytes())


#print 'Transcoder:'
#print len(DashTranscoder().transcode('foo').tobytes())
