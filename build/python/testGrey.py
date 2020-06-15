import pylightdb
from pylightdb import *

x  = SpatiotemporalRange(0,0)
y  = SpatiotemporalRange(0,0)
z  = SpatiotemporalRange(0,0)
vol = Volume(x,y,z)
geo = EquirectangularGeometry(0,0)
env = LocalEnvironment()
optimizer = HeuristicOptimizer(env)
coordinator = Coordinator()
grey = Greyscale()
query = Load("/home/pranay99/lightdb/test/resources/test-pattern.h264", {"Volume":vol, "Projection":geo}).Map(grey).Save("/home/pranay99/testGrey.hevc")
coordinator.Execute(query.query(), optimizer)
