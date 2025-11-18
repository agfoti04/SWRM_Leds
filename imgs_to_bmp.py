import cv2
#note: code for changing list of images with unique names into 64x64 bitmaps
#---------EDIT PARAMETERS HERE---------------
img_folder = 'folder/of/images'
img_names = ['john_doe', 'jane_doe','lorem_ipsum']
img_type = 'png'
destination_folder = 'where/you/want/your/bitmaps/saved'
#-------------------------------------------------------
for img_name in img_names:
    frame = cv2.imread(f'{img_folder}/{img_name}.{img_type}')
    square = cv2.resize(frame, (64,64))
    cv2.imwrite(f'{destination_folder}/{img_name}.bmp',square)