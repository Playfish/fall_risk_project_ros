#include <stdlib.h>
#include <ros/ros.h>
#include <remote_command_server/RemoteCmdSrv.h>
#include <QObject>
#include <QProcess>
#include <QString>

QProcess* myProcess = new QProcess(new QObject());

int runSysCommand(const char* cmd);

bool executeCommand(remote_command_server::RemoteCmdSrv::Request &req, remote_command_server::RemoteCmdSrv::Response &res)
{
//    if(req.cmd_name == req.CMD_AMCL)
//    {
//        if(req.cmd_action == req.START)
//        {
////            QString program = "roslaunch";
////            QStringList arguments;
////            arguments << "uvc_camera_image"<<"uvc_camera_start.launch";

////            for(int i=0; i<arguments.size();i++) {
////                std::cout<<arguments.at(i).toStdString()<<std::endl;
////            }

////            std::cout<<program.toStdString()<<std::endl;
////            myProcess->start(program, arguments);

////            ROS_INFO("STATE:%d",myProcess->state());
//            ROS_INFO("amcl start");
////            ROS_INFO("Process started with PID :%d",myProcess->pid());

//            res.cmd_status = res.CMD_SUCCESS;
////            res.cmd_PID = myProcess->pid();
//        }
//        else if(req.cmd_action == req.STOP)
//        {
////            myProcess->terminate();

////            ROS_INFO("STATE:%d",myProcess->state());
////            ROS_INFO("Killing process PID: %d",myProcess->pid());
//            ROS_INFO("amcl stop");

//            res.cmd_status = res.CMD_SUCCESS;
//        }
//    }
//    else if(req.cmd_name == req.CMD_GMAPPING)
//    {
//        if(req.cmd_action == req.START)
//        {
////            QString program = "roslaunch";
////            QStringList arguments;
////            arguments << "uvc_camera_image"<<"uvc_camera_start.launch";

////            for(int i=0; i<arguments.size();i++) {
////                std::cout<<arguments.at(i).toStdString()<<std::endl;
////            }

////            std::cout<<program.toStdString()<<std::endl;
////            myProcess->start(program, arguments);

////            ROS_INFO("STATE:%d",myProcess->state());
//            ROS_INFO("gmapping start");
////            ROS_INFO("Process started with PID :%d",myProcess->pid());

////            res.cmd_status = res.CMD_SUCCESS;
////            res.cmd_PID = myProcess->pid();
//        }
//        else if(req.cmd_action == req.STOP)
//        {
////            myProcess->terminate();

////            ROS_INFO("STATE:%d",myProcess->state());
////            ROS_INFO("Killing process PID: %d",myProcess->pid());
//            ROS_INFO("gmapping stop");
//        }
//    }

    ROS_INFO("STATE:%d",myProcess->state());

    if(myProcess->state() != 0)
        myProcess->terminate();

    if(req.cmd_name == req.CMD_AMCL)
    {
        QString program = "roslaunch";
        QStringList arguments;
        arguments << "fallrisk_turtlebot_navigation"<<"amcl_navigtaion.launch";

        myProcess->start(program, arguments);

        ROS_INFO("STATE:%d",myProcess->state());

        myProcess->waitForStarted(3000);

        if(myProcess->state() == 2)
        {
            res.cmd_status = res.CMD_SUCCESS;
            res.cmd_PID = myProcess->pid();
            ROS_INFO("amcl started");
        }
        else
        {
            myProcess->terminate();
            res.cmd_status = res.CMD_FAILURE;
            ROS_INFO("amcl failed to start");
        }
    }
    else if(req.cmd_name == req.CMD_GMAPPING)
    {
        QString program = "roslaunch";
        QStringList arguments;
        arguments << "fallrisk_turtlebot_navigation"<<"turtlebot_gmapping.launch";

        myProcess->start(program, arguments);

        myProcess->waitForStarted(3000);

        ROS_INFO("STATE:%d",myProcess->state());
        if(myProcess->state() == 2)
        {
            res.cmd_status = res.CMD_SUCCESS;
            res.cmd_PID = myProcess->pid();
            ROS_INFO("gmapping started");
        }
        else
        {
            myProcess->terminate();
            res.cmd_status = res.CMD_FAILURE;
            ROS_INFO("gmapping failed to start");
        }
    }

    return true;
}


int main(int argc, char **argv)
{
  ros::init(argc, argv, "checklist_status_server");
  ros::NodeHandle nh_;

  ros::ServiceServer cmd_server = nh_.advertiseService("remote_command",executeCommand);
  ROS_INFO("Ready to respond to remote command:");
  ros::spin();

  return 0;
}
