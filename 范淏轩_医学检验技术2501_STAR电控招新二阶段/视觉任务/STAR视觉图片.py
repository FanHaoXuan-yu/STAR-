import cv2
import numpy as np
import matplotlib.pyplot as plt
'''导入库'''

def detect_green_with_centroids():
    img = cv2.imread('star_picture.jpg')
    '''定义函数，读图'''

    img_rgb=cv2.cvtColor(img,cv2.COLOR_BGR2RGB)
    '''转换颜色格式'''

    hsv=cv2.cvtColor(img,cv2.COLOR_BGR2HSV)
    '''转换为HSV颜色空间，准备识别'''

    lower_green=np.array([35,50,50])
    upper_green=np.array([85,255,255])
    '''定义绿色'''

    mask=cv2.inRange(hsv,lower_green,upper_green)
    result=cv2.bitwise_and(img_rgb,img_rgb,mask=mask)
    '''创建绿色掩码并提取绿色'''

    contours, _ = cv2.findContours(mask, cv2.RETR_EXTERNAL, cv2.CHAIN_APPROX_SIMPLE)
    '''寻找绿色边框'''

    img_with_marks=img_rgb.copy()
    '''创建图片拷贝用于标记'''

    centroid_list=[]
    for i,contour in enumerate(contours):
        area=cv2.contourArea(contour)
        if area<100:
            continue

        M=cv2.moments(contour)
        if M["m00"]!=0:
            cx=int(M["m10"]/M["m00"])
            cy=int(M["m01"]/M["m00"])
            centroid_list.append((cx,cy))
            '''计算重心'''
            cv2.circle(img_with_marks,(cx,cy),8,(255,0,0),-1)
            cv2.putText(img_with_marks,str(i+1),(cx-10,cy-15),cv2.FONT_HERSHEY_SIMPLEX,0.7,(255,0,0),2)
            '''标记重心'''

    plt.figure(figsize=(15,5))

    plt.subplot(1,3,1)
    plt.imshow(img_rgb)
    plt.title('Original')
    plt.axis('off')

    plt.subplot(1,3,2)
    plt.imshow(result)
    plt.title('Green')
    plt.axis('off')

    plt.subplot(1,3,3)
    plt.imshow(img_with_marks)
    plt.title('Marks')
    plt.axis('off')

    plt.tight_layout()
    plt.show()
    '''显示'''

    print("检测到的重心坐标：")
    for i,(cx,cy) in enumerate(centroid_list):
        print(f"色块{i+1}:({cx},{cy})")
    return mask,result,centroid_list
detect_green_with_centroids()