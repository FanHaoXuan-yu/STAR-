import cv2
import numpy as np
'''导入库'''

def process_green_in_video():#主函数
    cap = cv2.VideoCapture('star_video.mp4')#创建对象

    while True:#读取，处理并显示视频的每一帧
        ret , frame =cap.read()#读取
        if not ret:break#视频播放完后结束

        processed_frame = detect_green_in_frame(frame)#处理

        cv2.imshow('Video',processed_frame)#显示
        if cv2.waitKey(1)&0xFF == ord('q'):break#视频的自动结束退出与按下q键手动退出

    cap.release()
    cv2.destroyAllWindows()
    '''清理资源'''

def detect_green_in_frame(frame):#副函数，用于处理
    hsv = cv2.cvtColor(frame, cv2.COLOR_BGR2HSV)
    '''转换为HSV颜色空间，准备识别'''

    lower_green = np.array([35, 50, 50])
    upper_green = np.array([85, 255, 255])
    '''定义绿色'''

    mask = cv2.inRange(hsv, lower_green, upper_green)
    result = cv2.bitwise_and(frame, frame, mask=mask)
    '''创建绿色掩码并提取绿色'''

    return result

process_green_in_video()