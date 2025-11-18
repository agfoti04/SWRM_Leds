import cv2
import os


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
        cv2.imwrite(f'{destination}/{frame_number}.bmp', square) #write as bitmap, to write as image use ".jpg" or ".png"

        # #early stopping
        # if cv2.waitKey(1) & 0xFF == ord("q"):
        #     break
        frame_number += 1

    #stopping at end of video
    else:
        break
    
print(f"{(frame_number - start)  // (30//fps)} frame(s) extracted from {video_name}") #could be incorrect count

#releasing memory
cap.release()
cv2.destroyAllWindows()
