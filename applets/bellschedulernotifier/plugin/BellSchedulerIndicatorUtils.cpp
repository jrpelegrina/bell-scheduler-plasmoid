/*
 * Copyright (C) 2015 Dominik Haumann <dhaumann@kde.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */
#include "BellSchedulerIndicatorUtils.h"

#include <QFile>
#include <QDateTime>
#include <QFileInfo>
#include <QRegularExpression>
#include <QStandardPaths>
#include <QDebug>
#include <QTextStream>
#include <QJsonObject>
#include <QList>

#include <n4d.hpp>
#include <variant.hpp>
#include <json.hpp>

#include <tuple>
#include <sys/types.h>

using namespace edupals;
using namespace std;


BellSchedulerIndicatorUtils::BellSchedulerIndicatorUtils(QObject *parent)
    : QObject(parent)
       
{
    client = new n4d::Client("https://localhost",9779);
    //variant::Variant bell_info =variant::Variant::create_array(0);
     BELLS_TOKEN.setFileName("tmp/.BellScheduler/bellscheduler-token");
  
}    


string BellSchedulerIndicatorUtils::getFormatHour(int hour,int minute){

    std::stringstream shour;
    shour<<hour;
    std::stringstream sminute;
    sminute<<minute;
    string format_hour="";
    string format_minute="";
  
    if (hour<10){
        format_hour='0'+shour.str();
    }else{
        format_hour=shour.str();

    }

    if (minute<10){
        format_minute='0'+sminute.str();
    }else{
        format_minute=sminute.str();

    }

   string format_time=format_hour+":"+format_minute;
   return format_time;

}



variant::Variant BellSchedulerIndicatorUtils::readToken(){


    variant::Variant tmp =variant::Variant::create_array(0);

    variant::Variant bell_info = client->call("BellSchedulerManager","read_conf");

    cout << bell_info << endl ;

    cout<<bell_info["status"]<<endl;
    cout << "----------" << endl;
    QFile file("/tmp/.BellScheduler/bellscheduler-token");
    //QFile file("/tmp/.BellScheduler/bellscheduler-token");

    file.open(QIODevice::ReadOnly);
    QString s;
    QTextStream s1(&file);
 
    QString line;
    QStringList bells_id;
    while (s1.readLineInto(&line)) {
        bells_id.push_back(line);
    } 
    file.close();
   // qDebug()<<"Lista de bells_id" << bells_id;
    
    for(int i=0 ; i < bells_id.length() ; i++){
        
        variant::Variant info=variant::Variant::create_struct();  
        //info["hour"]=bell_info["data"][bellId]["hour"].get_string();
        string bellId=bells_id[i].toStdString();
        cout << bellId << endl;
        int hour=bell_info["data"][bellId]["hour"].get_int32();
        int minute=bell_info["data"][bellId]["minute"].get_int32();
        string format_hour=getFormatHour(hour,minute);
        
        int duration=bell_info["data"][bellId]["play"]["duration"];

        clog<<duration<<endl;
        info["bellId"]=bellId;
        info["name"]=bell_info["data"][bellId]["name"].get_string();
        info["hour"]=format_hour;
        info["duration"]=duration;
        info["bellPID"]="";
        tmp.append(info);
        
    }
        
 
    clog<<tmp<<endl;

    qDebug()<<"fin";
    return tmp;


}

void BellSchedulerIndicatorUtils::getBellInfo(){

    variant::Variant token_content = readToken();
    cout << "#############" << endl;
    cout << token_content << endl;

 //   qDebug()<<"vacia";
    if (bellsInfo.count()>0){
        for (int i=0;i<bellsInfo.count();i++){
   //         qDebug()<<"3";
            if (!bellsId.contains(QString::fromStdString(bellsInfo[i]["bellId"]))){
                bellsId.push_back(QString::fromStdString(bellsInfo[i]["bellId"]));
            }    
        }
    } 
    
   for (int i=0;i<token_content.count();i++){
     //   qDebug()<<"5";
        QString id=QString::fromStdString(token_content[i]["bellId"]);
        if (!bellsId.contains(id)){
           // qDebug()<<"6";
            bellsInfo.append(token_content[i]);
            
        }   
    }   
    qDebug()<<"9";
    qDebug()<<"LISTA DE ALARMAS SONANDO";
    cout << bellsInfo << endl;

}


std::tuple<QList<QJsonObject>, QStringList> BellSchedulerIndicatorUtils::getBellPid(){

    QStringList bellPid;
    QList<QJsonObject>pidInfo;

    QProcess process;
    QString cmd="ps -ef | grep 'ffplay -nodisp -autoexit' | grep -v 'grep'";
    
    process.start("/bin/sh", QStringList()<< "-c" 
                       << cmd);
    process.waitForFinished(-1);
    QString stdout=process.readAllStandardOutput();
    QString stderr=process.readAllStandardError();
  //  qDebug()<<stdout;
    QStringList lst=stdout.split("\n");
 //   qDebug()<<"############SPLIT###########3"<<lst;

    if (lst.length()>0){
        for (int i=0;i<lst.length();i++){
            QStringList tmp_list;
            QJsonObject tmp_pid{
                {"bellId",""},
                {"pidParent",""},
                {"bellPID",""}

            };
            int cont=0;
            QStringList processed_line=lst[i].split(" ");
   //         qDebug()<< "##############PROCESADO LINEA###########"<<processed_line;
            if (processed_line.length()>=10){
                for (int i=0;i<processed_line.length();i++){
                    if (processed_line[i].compare("")!=0){
                        tmp_list.push_back(processed_line[i]);

                    }
                }    
     //           qDebug()<<"#########   TMP LIST"<<tmp_list;
                processed_line=tmp_list;
                if (processed_line[7].compare("/bin/bash")==0){
                    if (processed_line[9].compare("check_holiday")==0){
                        tmp_pid["bellId"]=processed_line[13];
                    }else{
                        tmp_pid["bellId"]=processed_line[11];

                    }
                    tmp_pid["pidParent"]=processed_line[1];
                    if (pidInfo.length()>0){
                        for (int i=0;i<pidInfo.length();i++){
                            if (pidInfo[i]["bellId"]==tmp_pid["bellId"]){
                                cont=cont+1;

                            }

                        }
                    }
                    if (cont==0){
                        pidInfo.append(tmp_pid);

                    }    

                }else{
      //              qDebug()<<"LINEA !=/bin/bash";
                    for (int i=0; i<pidInfo.length();i++){
        //                qDebug()<< processed_line[2];
        //                qDebug()<< pidInfo[i]["pidParent"].toString();
                        if (processed_line[2].compare(pidInfo[i]["pidParent"].toString())==0){
                            pidInfo[i]["bellPID"]=processed_line[1];
                        }

                        bellPid.append(processed_line[1]);    
                    }

                


                }    
    //            qDebug()<<"############## PID_INFO"<<pidInfo;
    //            qDebug()<<"############## TMP PID"<<tmp_pid;
    //            qDebug()<<"############## BELL PID"<<bellPid;
                
          }


        }
    }    
   
  //  qDebug()<<"########## INFO##########3";
    return {pidInfo,bellPid};

}

QStringList BellSchedulerIndicatorUtils::areBellsLive(){

    auto[pidInfo,bellPid]=getBellPid();

    QStringList removeBells;

 //  qDebug()<<"VIENDO SI HAY ALARMAS VIVAS";
  //  qDebug()<<"NUMERO DE PIDS"<<bellPid.size();

    if (bellPid.size()>0){
    //    qDebug()<<"NUMERO DE ALARMAS VIVAS"<<bellsInfo.count();
        for (int i=bellsInfo.count()-1;i>=0;i--){
      //      qDebug()<<"VIENDO SI HAY ALARMAS MUERTAS";
            QString bellPID=QString::fromStdString(bellsInfo[i]["bellPID"]);
        //    qDebug()<<"ARE LIVE??"<<bellPID;
            if (bellPID!=""){
                if (!bellPid.contains(bellPID)){
          //          qDebug()<<"BELL TERMINADA"<<bellPID;
                    removeBells.push_back(QString::fromStdString(bellsInfo[i]["bellId"]));
                 } 

            }

        }
    }    

    qDebug()<<"BORRADO DE ALARMAS"<<removeBells;    
    return removeBells;    

}

void BellSchedulerIndicatorUtils::linkBellPid(){

   // qDebug()<<"ASOCIANDO PID";
    int cont=0;
    auto[pidInfo,bellPid]=getBellPid();

  
    for (int i=0;i<bellsInfo.count();i++){
        QString bellPID=QString::fromStdString(bellsInfo[i]["bellPID"]);
        if (bellPID==""){
     //       qDebug()<<"contando";
            cont=cont+1;
        }
    }
   // qDebug()<<"CONTADOR DE PID";
    //qDebug()<<cont;
    if (cont>0){
      //  qDebug()<<"ASOCIANDO PID START";
        int pidInfoLength=sizeof(pidInfo)/sizeof(pidInfo[0]);
        //qDebug()<<"NUMERO PID INFO"<<pidInfoLength;
        //qDebug()<<"SIZE PID INFO"<<pidInfo.size();
        for (int i=0; i<pidInfo.size();i++){
            
                for (int j=0;j<bellsInfo.count();j++){
                    QString bellID=QString::fromStdString(bellsInfo[j]["bellId"]);
          //          qDebug()<<"#######3BellID"<<bellID;
                    QString bellID2=pidInfo[i]["bellId"].toString();
            //        qDebug()<<"#########3BellID2"<<bellID2;
                    if (bellID.compare(bellID2)==0){
              //          qDebug()<<"matc";
                        QString bellPID=pidInfo[i]["bellPID"].toString();
                        bellsInfo[j]["bellPID"]=bellPID.toStdString();
                    }

                }
            
        }

    }
    qDebug()<<"RESULTADO LINK";
    cout << bellsInfo << endl ; 

}

bool BellSchedulerIndicatorUtils::isTokenUpdated(){


    qDebug()<<"CHECKEANDO TOKEN";
    tokenUpdated=false;

    QDateTime currentTime=QDateTime::currentDateTime();
    QDateTime lastModification=QFileInfo(BELLS_TOKEN).lastModified();    

    qint64 millisecondsDiff = lastModification.msecsTo(currentTime);


    if (millisecondsDiff<MOD_FRECUENCY){
        tokenUpdated=true;
    }
    qDebug()<<"TOKEN UPDATE:"<<tokenUpdated;
    return tokenUpdated;

}

void BellSchedulerIndicatorUtils::stopBell(){

    client->call("BellSchedulerManager","stop_bell");
    
}