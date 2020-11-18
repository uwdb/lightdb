import pylightdb
import numpy as np
import cv2 as cv
from PIL import Image
from pylightdb import *

x  = SpatiotemporalRange(0,0)
y  = SpatiotemporalRange(0,0)
z  = SpatiotemporalRange(0,0)
vol = Volume(x,y,z)
geo = EquirectangularGeometry(0,0)
env = LocalEnvironment()
optimizer = HeuristicOptimizer(env)
coordinator = Coordinator()
def rgb2gray(rgb):
  rgb_weights = [0.2989, 0.5870, 0.1140]
  grayscale_image = np.dot(rgb[...,:3], rgb_weights)
  rgb[:,:,:] = grayscale_image[:,:,np.newaxis]
  return rgb
grey = PythonGreyscale(rgb2gray)
query = Load("/home/pranay99/colorsrc.h264", {"Volume":vol, "Projection":geo}).Map(grey).Save("/home/pranay99/lambdaTest.h264")
coordinator.Execute(query.query(), optimizer)