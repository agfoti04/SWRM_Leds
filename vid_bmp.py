import cv2
import os
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

# video_path = ""
video_path = r"C:\Hannah\CANR_SUMMER\seedlings\Code\gopro1.mp4"
video_name = "test"
destination_folder = r"C:\Hannah\TAMU\Fall 2025\SWRM"
fps = 10 # set to desired fps, this assumes og vid is 30 fps
cap = cv2.VideoCapture(video_path) 

start = 1 # set desired start and end of video (input in seconds)
start *= 30
# end = int(cap.get(cv2.CAP_PROP_FRAME_COUNT))
end = 2 #uncomment code if setting end of video 
end *= 30 
frame_number = 0

#creating folder for frames
count = 0
while(True):
    try:
        if (count == 0):
            destination = f'{destination_folder}/{video_name}'
        else:
            destination = f'{destination_folder}/{video_name}{count}'
        os.mkdir(destination)    
        break

    except FileExistsError:
        count += 1

#writing images
while cap.isOpened():
    
    success, frame = cap.read() #returns 2 values, boolean and frame
    # print(frame_number)
    # cropping video length
    if (frame_number <= start):
        frame_number += 1
        continue
    
    if (frame_number >= end):
        break

    # if you want to capture every other frame etc
    if (frame_number % (30//fps) != 0): 
        frame_number += 1
        continue
    
    if success:
        square = cv2.resize(frame, (64,64)) #resize to 64 bits
        bitmap = img_to_array(square)
        write_arr_csv(bitmap,f'{destination}/{video_name}_{frame_number}')
        frame_number += 1

    #stopping at end of video
    else:
        break
    
print(f"{(frame_number - start)  // (30//fps)} frame(s) extracted from {video_name}") #could be incorrect count

#releasing memory
cap.release()
cv2.destroyAllWindows()
