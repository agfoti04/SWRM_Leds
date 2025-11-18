import cv2
#note: code for changing list of images with unique names into 64x64 bitmaps
#---------EDIT PARAMETERS HERE---------------
img_folder = 'folder/of/images'
img_names = ['john_doe', 'jane_doe','lorem_ipsum']
img_type = 'png'
destination_folder = 'where/you/want/your/bitmaps/saved' 
# img_folder = r'C:\Hannah\TAMU\Fall 2025\SWRM'
# img_names = ['bonfire']
# img_type = 'jpg'
# destination_folder = r'C:\Hannah\TAMU\Fall 2025\SWRM'
#(0,0) = no split, (1,0) = L/R split, (1,1) = T/B split
split = (1,1) # tuple to store how to split the image

#-------------------------------------------------------
for img_name in img_names:
    frame = cv2.imread(f'{img_folder}/{img_name}.{img_type}')
    

    if (split[0] == 1):
        h,w,ch = frame.shape
        if (split[1] == 0): #L/R split
            img1 = frame[:,0:(w//2)]
            img2 = frame[:,(w//2):]
        else: # T/B split
            img1 = frame[0:(h//2), :]    
            img2 = frame[(h//2):, :]

        img1 = cv2.resize(img1,(64,64))
        img2 = cv2.resize(img2,(64,64))
        cv2.imwrite(f'{destination_folder}/{img_name}_a.bmp',img1)
        cv2.imwrite(f'{destination_folder}/{img_name}_b.bmp',img2)
    else:
        square = cv2.resize(frame, (64,64))
        cv2.imwrite(f'{destination_folder}/{img_name}.bmp',square)