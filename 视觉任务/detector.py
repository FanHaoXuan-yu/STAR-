import cv2
import os
import time
from ultralytics import YOLO


def optimized_detection(video_path="熊出没.mp4"):

    print("=" * 60)
    print("人员行为检测")
    print("=" * 60)

    # 优化配置
    CONFIG = {
        'detect_interval': 1,  # 每5帧检测一次
        'process_scale': 1.0,  # 处理图像大小
        'display_scale': 1.0,  # 显示窗口缩小到80%
        'model_size': 'n',  # n=最小, s=小, m=中, l=大, x=超大

        # 清洁模式
        'save_output': False,  # 不保存输出视频
        'save_report': False,  # 不保存报告
        'clear_cache': True,  # 自动清理缓存

        # 性能显示
        'show_fps': True,  # 显示实时FPS
        'show_stats': True,  # 显示统计数据
    }

    # 1. 加载优化模型
    model_name = f'yolov8{CONFIG["model_size"]}.pt'
    print(f"[1/3] 加载模型: {model_name}")
    try:
        model = YOLO(model_name)
        print("模型加载成功")
    except Exception as e:
        print(f"模型加载失败，使用最小模型: {e}")
        model = YOLO('yolov8n.pt')

    # 2. 打开视频
    print(f"[2/3] 打开视频: {video_path}")
    cap = cv2.VideoCapture(video_path)

    if not cap.isOpened():
        print(f"无法打开视频文件")
        print("请检查:")
        print(f"  1. 文件是否存在: {os.path.exists(video_path)}")
        print(f"  2. 文件路径是否正确")
        input("\n按Enter键退出...")
        return

    # 获取视频信息
    orig_width = int(cap.get(cv2.CAP_PROP_FRAME_WIDTH))
    orig_height = int(cap.get(cv2.CAP_PROP_FRAME_HEIGHT))
    fps = cap.get(cv2.CAP_PROP_FPS)
    total_frames = int(cap.get(cv2.CAP_PROP_FRAME_COUNT))

    # 计算优化尺寸
    proc_width = int(orig_width * CONFIG['process_scale'])
    proc_height = int(orig_height * CONFIG['process_scale'])
    disp_width = int(orig_width * CONFIG['display_scale'])
    disp_height = int(orig_height * CONFIG['display_scale'])

    print("视频信息:")
    print(f"  原始尺寸: {orig_width}x{orig_height}")
    print(f"  处理尺寸: {proc_width}x{proc_height} (加速{CONFIG['process_scale'] * 100:.0f}%)")
    print(f"  显示尺寸: {disp_width}x{disp_height}")
    print(f"  帧率: {fps:.1f} FPS")
    print(f"  总帧数: {total_frames}")
    print(f"  检测间隔: 每{CONFIG['detect_interval']}帧检测一次")
    print("-" * 40)

    # 行为颜色定义
    BEHAVIOR_COLORS = {
        'standing': (0, 255, 0),  # 绿色
        'sitting': (0, 165, 255),  # 橙色
        'walking': (255, 0, 0),  # 蓝色
    }

    # 窗口名称
    WINDOW_NAME = '人员行为检测 - 高速模式 (按Q退出)'

    # 性能统计
    frame_count = 0
    total_time = 0
    detection_time = 0
    stats = {'standing': 0, 'sitting': 0, 'walking': 0, 'unknown': 0, 'total': 0}

    print("[3/3] 开始处理视频...")
    print("-" * 40)
    print("操作说明:")
    print("  按 Q 键: 退出程序")
    print("  按 S 键: 暂停/继续")
    print("  按 F 键: 显示/隐藏FPS")
    print("-" * 40)

    paused = False
    show_fps = CONFIG['show_fps']

    try:
        while cap.isOpened():
            if not paused:
                frame_start = time.time()

                ret, frame = cap.read()
                if not ret:
                    print("视频播放结束")
                    break

                frame_count += 1

                # 1. 预处理：缩小图像以加速
                if CONFIG['process_scale'] != 1.0:
                    process_frame = cv2.resize(frame, (proc_width, proc_height))
                else:
                    process_frame = frame

                # 2. 间隔检测（核心优化）
                boxes_data = []
                if frame_count % CONFIG['detect_interval'] == 0:
                    detect_start = time.time()

                    # YOLO检测（使用缩小后的图像）
                    results = model(process_frame,
                                    classes=[0],
                                    verbose=False,
                                    conf=0.5,
                                    imgsz=640)

                    detection_time += time.time() - detect_start

                    if results[0].boxes is not None:
                        for box in results[0].boxes:
                            # 获取坐标（注意：这是缩小后的坐标）
                            x1, y1, x2, y2 = map(int, box.xyxy[0])
                            conf = float(box.conf[0])

                            if conf < 0.3:  # 过滤低置信度检测
                                continue

                            # 转换回原始尺寸坐标
                            scale_x = orig_width / proc_width
                            scale_y = orig_height / proc_height

                            x1_orig = int(x1 * scale_x)
                            y1_orig = int(y1 * scale_y)
                            x2_orig = int(x2 * scale_x)
                            y2_orig = int(y2 * scale_y)

                            # 行为判断
                            h = y2_orig - y1_orig
                            w = x2_orig - x1_orig

                            if w > 0:
                                ratio = h / w
                                if ratio > 2.8:
                                    behavior = 'standing'
                                elif ratio > 2.0:
                                    behavior = 'walking'
                                elif ratio > 1.6:
                                    behavior = 'sitting'
                            else:
                                behavior = 'sitting'

                            # 更新统计
                            stats[behavior] += 1
                            stats['total'] += 1

                            boxes_data.append({
                                'coords': (x1_orig, y1_orig, x2_orig, y2_orig),
                                'behavior': behavior,
                                'confidence': conf
                            })

                # 3. 绘制检测结果
                for box_info in boxes_data:
                    x1, y1, x2, y2 = box_info['coords']
                    behavior = box_info['behavior']
                    conf = box_info['confidence']

                    color = BEHAVIOR_COLORS.get(behavior, (128, 128, 128))

                    # 绘制边界框
                    cv2.rectangle(frame, (x1, y1), (x2, y2), color, 2)

                    # 绘制标签
                    label = f"{behavior} {conf:.2f}"
                    (label_w, label_h), baseline = cv2.getTextSize(
                        label, cv2.FONT_HERSHEY_SIMPLEX, 0.6, 2
                    )

                    # 标签背景
                    cv2.rectangle(frame,
                                  (x1, y1 - label_h - baseline - 5),
                                  (x1 + label_w, y1),
                                  color, -1)

                    # 标签文字
                    cv2.putText(frame, label,
                                (x1, y1 - baseline - 5),
                                cv2.FONT_HERSHEY_SIMPLEX, 0.6, (255, 255, 255), 2)

                # 4. 性能信息显示
                frame_time = time.time() - frame_start
                total_time += frame_time
                current_fps = 1.0 / frame_time if frame_time > 0 else 0

                # 左上角：帧率和进度
                if show_fps:
                    fps_text = f"FPS: {current_fps:.1f}"
                    cv2.putText(frame, fps_text, (10, 30),
                                cv2.FONT_HERSHEY_SIMPLEX, 0.8, (0, 255, 255), 2)

                progress = frame_count / total_frames * 100
                progress_text = f"进度: {progress:.1f}% ({frame_count}/{total_frames})"
                cv2.putText(frame, progress_text, (10, 60),
                            cv2.FONT_HERSHEY_SIMPLEX, 0.7, (255, 255, 255), 2)

                # 右上角：行为统计
                if CONFIG['show_stats']:
                    stats_text = f"站立: {stats['standing']} 坐: {stats['sitting']} 走: {stats['walking']}"
                    text_width = frame.shape[1]
                    text_x = text_width - 350 if text_width > 400 else 10
                    cv2.putText(frame, stats_text, (text_x, 30),
                                cv2.FONT_HERSHEY_SIMPLEX, 0.7, (255, 255, 0), 2)

                # 5. 调整显示尺寸
                if CONFIG['display_scale'] != 1.0:
                    display_frame = cv2.resize(frame, (disp_width, disp_height))
                else:
                    display_frame = frame

                # 6. 显示
                cv2.imshow(WINDOW_NAME, display_frame)

            # 7. 键盘控制
            key = cv2.waitKey(1) & 0xFF

            if key == ord('q'):
                print("用户中断处理")
                break
            elif key == ord('s'):
                paused = not paused
                print(f"{'暂停' if paused else '继续'}播放")
            elif key == ord('f'):
                show_fps = not show_fps
                print(f"{'显示' if show_fps else '隐藏'}FPS")

            # 8. 终端进度显示
            if frame_count % 50 == 0 and not paused:
                avg_fps = frame_count / total_time if total_time > 0 else 0
                print(f"处理进度: {progress:.1f}% | 平均FPS: {avg_fps:.1f} | 检测人数: {stats['total']}")

    except KeyboardInterrupt:
        print("\n程序被中断")
    finally:
        # 清理资源
        cap.release()
        cv2.destroyAllWindows()

        # 性能报告
        print("\n" + "=" * 60)
        print("处理完成 - 性能报告")
        print("=" * 60)

        if total_time > 0:
            avg_fps = frame_count / total_time
            detect_percent = (detection_time / total_time * 100) if total_time > 0 else 0

            print(f"处理帧数: {frame_count}/{total_frames}")
            print(f"总时间: {total_time:.1f}秒")
            print(f"平均FPS: {avg_fps:.1f}")
            print(f"检测时间占比: {detect_percent:.1f}%")

        print("\n行为检测统计:")
        print(f"  总人数: {stats['total']}")
        print(f"  站立: {stats['standing']}")
        print(f"  坐姿: {stats['sitting']}")
        print(f"  行走: {stats['walking']}")
        print(f"  未知: {stats['unknown']}")

        # 清洁模式：不保存任何文件
        if CONFIG['clear_cache']:
            print("\n清洁模式：未生成任何文件")
            print("（视频未保存，报告未生成，缓存已跳过）")

        print("=" * 60)


def main():
    """主函数"""

    # 自动查找当前目录的视频
    video_files = []
    for file in os.listdir("."):
        if file.lower().endswith(('.mp4', '.avi', '.mov', '.mkv', '.flv', '.wmv')):
            video_files.append(file)

    if not video_files:
        print("在当前目录未找到视频文件！")
        print("请将视频文件放在与此Python文件相同的目录下")
        input("按Enter键退出...")
        return

    # 选择视频
    if len(video_files) == 1:
        video_path = video_files[0]
        print(f"找到视频: {video_path}")
    else:
        print("找到多个视频文件:")
        for i, v in enumerate(video_files, 1):
            try:
                size = os.path.getsize(v) / (1024 * 1024)
                print(f"  {i}. {v} ({size:.1f} MB)")
            except:
                print(f"  {i}. {v}")

        while True:
            try:
                choice = input(f"选择视频 (1-{len(video_files)}): ").strip()
                if not choice and len(video_files) == 1:
                    video_path = video_files[0]
                    break

                choice_num = int(choice)
                if 1 <= choice_num <= len(video_files):
                    video_path = video_files[choice_num - 1]
                    break
                else:
                    print(f"请输入 1 到 {len(video_files)} 之间的数字")
            except ValueError:
                print("请输入有效的数字")
            except KeyboardInterrupt:
                print("\n取消选择")
                return

    # 运行优化检测
    optimized_detection(video_path)

    input("\n按Enter键退出程序...")


if __name__ == "__main__":
    main()