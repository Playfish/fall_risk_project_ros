#include <stdlib.h>
#include <ros/ros.h>
#include <remote_command_server/RemoteCmdSrv.h>
#include <QObject>
#include <QProcess>
#include <QString>
#include "std_srvs/Empty.h"
#include <QFile>
#include <QTextStream>

QProcess* navigationProcess = new QProcess(new QObject());
QProcess* telepresenceProcess = new QProcess(new QObject());

int runSysCommand(const char* cmd);

bool executeCommand(remote_command_server::RemoteCmdSrv::Request &req, remote_command_server::RemoteCmdSrv::Response &res)
{

    ROS_INFO("Navigation Process State:%d",navigationProcess->state());
    ROS_INFO("Telepresence Process State:%d",telepresenceProcess->state());

    //Telepresence action is called on tab change, to avoid false trigger use cmd_action to confirm start of teleoperation

    if (req.cmd_name == req.CMD_TELEPRESENCE)
    {

        if(telepresenceProcess->state() == 0 && req.cmd_action == req.START)
        {
            QString program = "roslaunch";
            QStringList arguments;
            arguments << "fallrisk_bringup"<<"telepresenceTurtlebot.launch";

            telepresenceProcess->start(program, arguments);

            ROS_INFO("Telepresence Process State:%d",telepresenceProcess->state());

            telepresenceProcess->waitForStarted(3000);

            if(telepresenceProcess->state() == 2)
            {
                res.cmd_status = res.CMD_SUCCESS;
                res.cmd_PID = telepresenceProcess->pid();
                ROS_INFO("Telepresence started on turtlebot");
            }
            else
            {
                telepresenceProcess->terminate();
                res.cmd_status = res.CMD_FAILURE;
                ROS_INFO("Telepresence failed to start on turtlebot");
            }
        }
        else
        {
            telepresenceProcess->terminate();
            telepresenceProcess->waitForFinished(3000);
            res.cmd_status = res.CMD_SUCCESS;
            ROS_INFO("Telepresence Stopped on turtlebot");

        }

    }
    else {
        if(navigationProcess->state() != 0){

            if(req.cmd_name==req.CMD_AMCL){
                //Entering this condition means that user switched out of gmapping to AMCL.

                //@TODO: Prompt user to save map




                //Find the path to maps folder on turtlebot
                QProcess* findProcess = new QProcess(new QObject());
                QStringList findArguments;
                findArguments<<"find"<<"fallrisk_turtlebot_navigation";
                findProcess->start("rospack",findArguments);
                findProcess->waitForFinished();
                QString pathToMaps(findProcess->readAllStandardOutput());
                pathToMaps.remove(QChar('\n'));
                pathToMaps = pathToMaps.append("/maps/default");
                ROS_INFO_STREAM(pathToMaps.toStdString());
                delete findProcess;

                //run mapsaver command to save existing map as default.pm in the maps directory
                //using projected_map from octomap_server as it is more accurate in cluttered environment
                QProcess* mapsaver = new QProcess(new QObject());
                QString program = "rosrun";
                QStringList arguments;
                QString frameid="map:=/projected_map";
                arguments << "map_server"<<"map_saver"<<"-f"<<pathToMaps<<frameid;
//                ROS_INFO_STREAM(frameid.toStdString());
                ROS_INFO("Saving Map");
                mapsaver->start(program,arguments);
                bool saved = mapsaver->waitForFinished(10000);

                if(saved){
                    ROS_INFO("Map saved successfully, fixing the z coordinate in yaml file");

                    //2D map generated by octomap has z coordinate of origin as nan. this needs to be replaced by 0.0
                    QByteArray fileData;
                    QString filename = pathToMaps.append(".yaml");
//                    ROS_INFO_STREAM(filename.toStdString());
                    QFile file(filename);
                    file.open(QIODevice::ReadWrite | QIODevice::Text); // open for read and write
                    fileData = file.readAll(); // read all the data into the byte array
                    QString fileDataString(fileData);
//                    ROS_INFO_STREAM(fileDataString.toStdString());
                    fileDataString.replace("nan","0.000");
//                    ROS_INFO_STREAM(fileDataString.toStdString());
                    file.seek(0); // go to the beginning of the file
                    file.write(fileDataString.toUtf8()); // write the new text back to the file
                    file.close(); // close the file handle.
                    ROS_INFO("File updated successfully");
                }
                else
                {
                    ROS_INFO("Cannot read the file!");
                    ROS_INFO("Failed to save the map");
                }
                delete mapsaver;
            //octomap saves the binay file by itself. so there is no need to save the 3D map
            }

        navigationProcess->terminate();
        navigationProcess->waitForFinished(3000);
    }

    if(req.cmd_name == req.CMD_AMCL)
    {
        QString program = "roslaunch";
        QStringList arguments;
        arguments << "fallrisk_turtlebot_navigation"<<"amcl_navigtaion.launch";

        navigationProcess->start(program, arguments);

        ROS_INFO("STATE:%d",navigationProcess->state());

        navigationProcess->waitForStarted(3000);

        if(navigationProcess->state() == 2)
        {
            res.cmd_status = res.CMD_SUCCESS;
            res.cmd_PID = navigationProcess->pid();
            ROS_INFO("amcl started");
        }
        else
        {
            navigationProcess->terminate();
            navigationProcess->waitForFinished(3000);
            res.cmd_status = res.CMD_FAILURE;
            ROS_INFO("amcl failed to start");
        }
    }
    else if(req.cmd_name == req.CMD_GMAPPING)
    {
        //reset the octomap

        std_srvs::Empty emptySrv0;
        if (ros::service::call("/octomap_server/reset", emptySrv0))
        {
            ROS_INFO("Octomap is reset successfully!");
        }
        else
        {
            ROS_INFO("Failed to reset octomap!");
        }

        //Launch gmapping app on turtlebot

        QString program = "roslaunch";
        QStringList arguments;

        arguments << "fallrisk_turtlebot_navigation"<<"turtlebot_gmapping.launch";


        navigationProcess->start(program, arguments);

        navigationProcess->waitForStarted(3000);

        ROS_INFO("STATE:%d",navigationProcess->state());
        if(navigationProcess->state() == 2)
        {
            res.cmd_status = res.CMD_SUCCESS;
            res.cmd_PID = navigationProcess->pid();
            ROS_INFO("gmapping started");
        }
        else
        {
            navigationProcess->terminate();
            navigationProcess->waitForFinished(3000);
            res.cmd_status = res.CMD_FAILURE;
            ROS_INFO("gmapping failed to start");
        }
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

