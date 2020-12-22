import cv2
import os
import argparse


if __name__ == "__main__":
    parser = argparse.ArgumentParser()
    parser.add_argument('-img_dir', type=str,default="./data/lane_result/",help = 'input a int')
    parser.add_argument('-width', type=int, default=1080, help='input a int')
    parser.add_argument('-height', type=int, default=388, help='input a int')
    args = parser.parse_args()
    print(args)


    fps = 15
    size = (args.width, args.height)
    videoWriter = cv2.VideoWriter('output.mp4', cv2.VideoWriter_fourcc('M', 'P', 'E', 'G'), fps, size)

    files = os.listdir(args.img_dir)
    for f in files:
        if not f.endswith(".jpg"):
            continue
        img_path = os.path.join(args.img_dir,f)
        img = cv2.imread(img_path)
        img = cv2.resize(img,size)
        videoWriter.write(img)

    videoWriter.release()
