import cv2
import csv

def img_to_array(img):
    bitmap = [None] * 64 * 64
    for j in range(64): #col
        for i in range(64): #imread is bgr
            red = round(img[i][j][2] / 255 * 31)
            green = round(img[i][j][1] / 255 * 63)
            blue = round(img[i][j][0] / 255 * 31)
            pixel = (red << 11) | (green << 5) | blue
            bitmap[i + j * 64] = (f'0x{pixel:04X}')
    return bitmap

def write_arr_csv(bitmap,img_name):
    output_file = f'{img_name}.csv'

    with open(output_file, 'w', newline='') as csvfile:
        writer = csv.writer(csvfile)
        # loads 64 pixels per row
        for i in range(1,65):
            writer.writerow(bitmap[(i-1)*64:i*64]) 

#note: code for changing list of images with unique names into 64x64 bitmaps
#---------EDIT PARAMETERS HERE---------------
img_folder = 'folder/of/images'
img_names = ['john_doe', 'jane_doe','lorem_ipsum']
img_type = 'png'
destination_folder = 'where/you/want/your/bitmaps/saved'
#(0,0) = no split, (1,0) = L/R split, (1,1) = T/B split
split = (1,0) # tuple to store how to split the image

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
        
        bitmap = img_to_array(img1)
        write_arr_csv(bitmap,f'{destination_folder}/{img_name}_a')
        bitmap = img_to_array(img2)
        write_arr_csv(bitmap,f'{destination_folder}/{img_name}_b')
    else:
        square = cv2.resize(frame, (64,64))
        bitmap = img_to_array(square)
        write_arr_csv(bitmap,f'{destination_folder}/{img_name}')