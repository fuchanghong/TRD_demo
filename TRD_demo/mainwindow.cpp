#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "TRD_Tracker.h"

#define KEYESC 27
#define Nf 25   // number of features
#define Sd 350  // size of dictionary
#define mi 20   // parameter of feature extraction
#define StartFrame 0

//variables for input
//
QString InputImageAddress;
cv::Mat InputImageData, PreviousImageData;
QString img_dir;
QStringList images;
//QList<int> array;
cv::Rect inital_bounding_box;
TRD_Tracker mytracker(Nf,Sd,mi);
int frame_index = StartFrame;

bool cameraFlag = 0;
bool TrackingEnableFlag = 0;

//bool clicked=false;

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
}

MainWindow::~MainWindow()
{
    delete ui;
}

//void MainWindow::on_CameraButton_clicked()
//{
//        cv::VideoCapture cap = cv::VideoCapture(0);
//        cv::Mat frame;
//        cameraFlag = 1;
//        do
//        {
//            cap >> frame;
//            frame.copyTo(InputImageData);
//            cv::imshow("image",InputImageData);
//            // do tracking here

//        }while(cv::waitKey(10) != KEYESC);
//        cv::waitKey(0);
//        cameraFlag = 0;
//}

void MainWindow::on_Load_dataset_botton_clicked()
{
    frame_index = StartFrame;
    mytracker.TrackerClear();
    QString dataset_dir = QFileDialog::getExistingDirectory(this, tr("Open Directory"),
                                                    "/home",
                                                    QFileDialog::ShowDirsOnly
                                                    | QFileDialog::DontResolveSymlinks);
    img_dir = dataset_dir + "/img/";
    QString init_dir = dataset_dir + "/groundtruth_rect.txt";
    QFile file(init_dir);
     if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
         return;
     int nn = 0;
     QString txt_line;
     QTextStream in(&file);
        while (!in.atEnd() && nn < frame_index+1)
        {
           txt_line = in.readLine();
           nn ++;
        }
        file.close();
//     QString txt_line = file.readLine();
     txt_line.replace(","," ");
     QTextStream stream(&txt_line);
     int number_table[5] = {0,0,0,0,0};
     int ii = 0;
     int number;
     while (!stream.atEnd())
     {
             stream >> number;
             number_table[ii] = number;
             ii ++;
     }
     inital_bounding_box = cvRect(number_table[0],number_table[1],number_table[2],number_table[3]);
//     checkBoundary(InputImageData, inital_bounding_box);

     QDir directory(img_dir);
     images = directory.entryList(QStringList() << "*.png" << "*.jpg",QDir::Files);
     InputImageAddress = img_dir + images[frame_index];

     InputImageData = cv::imread(InputImageAddress.toStdString());

     if(InputImageData.empty())
     {
         InputImageData = cv::imread("Troll_Problem.jpg");
         cv::imshow("result",InputImageData);
     }
     else
     {
        mytracker.TrackerInitialization(InputImageData, inital_bounding_box);
        cv::rectangle(InputImageData, inital_bounding_box, cv::Scalar(0,255,0), 1, 8, 0 );
        cv::imshow("result",InputImageData);
        qDebug() << " sequence size = " << images.size();
     }
}

//void MainWindow::on_Load_init_file_botton_clicked()
//{
//    QString filename = QFileDialog::getOpenFileName();

//    QFile file(filename);
//     if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
//         return;

//    QString txt_line = file.readLine();
//    txt_line.replace(","," ");
//    QTextStream stream(&txt_line);

//    int number_table[5] = {0,0,0,0,0};
//    int ii = 0;
//    while (!stream.atEnd()) {
//        int number;
//        stream >> number;
//        number_table[ii] = number;
//        ii ++;
//    }
//    inital_bounding_box = cvRect(number_table[0],number_table[1],number_table[2],number_table[3]);
//}

void MainWindow::on_Tracking_start_botton_clicked()
{
    TrackingEnableFlag = 1;
    while(frame_index + 1 < images.size() && TrackingEnableFlag)
    {
        if(cv::waitKey(10) == KEYESC)
            break;
        frame_index = frame_index + 1;
        qDebug() << "----------------image number " << frame_index + 1 << "-----------------------";
        InputImageData.release();
        InputImageAddress = img_dir + images[frame_index];
        InputImageData = cv::imread(InputImageAddress.toStdString());
        mytracker.RunTracker(InputImageData);
        cv::rectangle(InputImageData, mytracker.results[frame_index-StartFrame], cv::Scalar(0,255,0), 1, 8, 0 );
        cv::imshow("result",InputImageData);
        cv::waitKey(1);
    }
    qDebug() << "--------------------------stop----------------------------------";
}

void MainWindow::on_Tracking_Pause_botton_clicked()
{
    TrackingEnableFlag = 0;
}

void MainWindow::on_Tracking_stop_botton_clicked()
{
    frame_index = StartFrame;
    TrackingEnableFlag = 0;
}

void MainWindow::on_Debug_SingleStep_clicked()
{
        if(frame_index + 1 < images.size())
        {
                frame_index = frame_index + 1;
                qDebug() << "----------------image number " << frame_index + 1 << "-----------------------";
                InputImageData.release();
                InputImageAddress = img_dir + images[frame_index];
                InputImageData.release();
                InputImageData = cv::imread(InputImageAddress.toStdString());
                mytracker.FeatureMatchingTest(InputImageData);
                cv::rectangle(InputImageData, mytracker.results[frame_index-StartFrame], cv::Scalar(0,255,0), 1, 8, 0 );
                cv::imshow("result",InputImageData);
        }
        else
        {
            qDebug() << "index number = " << frame_index << " sequence has been finished...";
        }
}

void MainWindow::on_Change_result_botton_clicked()
{
    QString s;
    QByteArray pchar;
    QString fileName = QFileDialog::getSaveFileName(this);
    if (fileName.isEmpty())
    {
        InputImageData = cv::imread("Troll_Problem.jpg");
        cv::imshow("result",InputImageData);
        qDebug() << "It is not a good idea...";
    }
    qDebug() << fileName;
    QFile myfile(fileName);
    myfile.open(QIODevice::WriteOnly|QIODevice::Append);
    for(int ii = 0; ii < images.size(); ii ++)
    {
        s = QString::number(mytracker.results[ii].x) + " " + QString::number(mytracker.results[ii].y) + " " + QString::number(mytracker.results[ii].width) + " " + QString::number(mytracker.results[ii].height) + "\n";
        pchar = s.toLatin1();
        myfile.write(pchar);
    }
    myfile.close();
    qDebug() << "results saved...";
}
