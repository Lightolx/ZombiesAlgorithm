#include <iostream>
#include <stack>

#include <opencv2/core.hpp>
#include <opencv2/imgcodecs.hpp>
#include <cv.hpp>

using std::cout;
using std::endl;

// 输入是单通道图像，输出是带label的单通道图像，每个像素块中的所有像素拥有相同的label
cv::Mat Flu(cv::Mat im) {
    // 对每个像素赋不同的label，因为是CV_8UC1，所以这里也决定了图像上最多有256个像素块,是个隐形bug
    cv::Mat image_label(im.size(), im.type(), cv::Scalar::all(0));

    int rows = im.rows;
    int cols = im.cols;
    int label = 0;
    for (int i = 0; i < rows; ++i) {
        uchar *data = im.ptr<uchar>(i);
        for (int j = 0; j < cols; ++j) {
            if (data[j] != 0) {  // 活人，变异成了僵尸，然后自爆去感染自己的上下左右4个邻居
                std::stack<cv::Point2i> vNeighborPixels;  // 还没自爆的僵尸集合
                vNeighborPixels.emplace(i, j);
                label++;  // 这一坨僵尸的病毒类型
                assert(label < 256);
                while (!vNeighborPixels.empty()) {
                    // 取出栈顶的元素，赋予它label(被传染的病毒的类型)
                    cv::Point2i curPixel = vNeighborPixels.top();

                    int x = curPixel.x;
                    int y = curPixel.y;
                    // 在这里并不做边界检查，因为在入栈时保证了所有入栈的像素都是在边界以内的
                    image_label.at<uchar>(x, y) = label;

                    // 自爆，变成死人
                    im.at<uchar>(x, y) = 0;

                    // 把栈顶元素pop出来，因为它已经自爆了
                    vNeighborPixels.pop();

                    // 检查自爆僵尸的4个neighbor, 如果是活人，就可以感染
                    if (x >= 1) {  // 可以往上扩展
                        if (im.at<uchar>(x - 1, y) != 0) {
                            vNeighborPixels.emplace(x - 1, y);
                        }
                    }

                    if (x <= rows - 2) {  // 可以往下扩展
                        if (im.at<uchar>(x + 1, y) != 0) {
                            vNeighborPixels.emplace(x + 1, y);
                        }
                    }

                    if (y >= 1) {  // 可以往左扩展
                        if (im.at<uchar>(x, y - 1) != 0) {
                            vNeighborPixels.emplace(x, y - 1);
                        }
                    }

                    if (y <= cols - 2) {  // 可以往右扩展
                        if (im.at<uchar>(x, y + 1) != 0) {
                            vNeighborPixels.emplace(x, y + 1);
                        }
                    }
                }
            }
        }
    }

    if (cv::countNonZero(im) > 0) {  // 还有人没死
        std::cerr << "Flu failed" << std::endl;
    }

    return image_label;
}


int main() {
//    cv::Mat im_initial = cv::imread("../ZebraStripe.png", CV_LOAD_IMAGE_GRAYSCALE);
    cv::Mat im_initial = cv::imread("../ZebraStripe.png");
    cv::Mat im;
    cv::cvtColor(im_initial, im, CV_BGR2GRAY);

    cv::imshow("ini", im_initial);
    cv::Mat im_seg = Flu(im);
    std::vector<cv::Vec3b> vColors(256);
    for (int i = 0; i < 256; ++i) {
        cv::Vec3b color;
        for (int j = 0; j < 3; ++j) {
            color.val[j] = rand() % 255;
        }

        vColors[i] = color;
    }

    cv::Mat image(im_seg.size(), CV_8UC3, cv::Scalar::all(0));
    for (int i = 0; i < im_seg.rows; ++i) {
        for (int j = 0; j < im_seg.cols; ++j) {
            if (im_seg.at<uchar>(i, j) != 0) {
                image.at<cv::Vec3b>(i, j) = vColors[im_seg.at<uchar>(i, j)];
            }
        }
    }

    cv::imshow("segmentation", image);
    cv::waitKey();

    return 0;
}