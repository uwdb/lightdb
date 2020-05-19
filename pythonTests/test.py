import pylightdb
from pylightdb import *

x  = SpatiotemporalRange(0,0)
y  = SpatiotemporalRange(0,0)
z  = SpatiotemporalRange(0,0)
vol = Volume(x,y,z)
geo = EquirectangularGeometry(2,1)
env = LocalEnvironment()
optimizer = HeuristicOptimizer(env)
coordinator = Coordinator()
query = Load("/home/pranay99/lightdb/test/resources/test-pattern.h264", {"Volume":vol, "Projection":geo}).Save("/home/pranay99/test.mp4")
coordinator.Execute(query.query(), optimizer)