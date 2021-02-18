def rgb2grey(rgb):
  rgb_weights = [0.2989, 0.5870, 0.1140]
  greyscale_image = np.dot(rgb[...,:3], rgb_weights)
  rgb[:,:,:] = greyscale_image[:,:,np.newaxis]
  return rgb