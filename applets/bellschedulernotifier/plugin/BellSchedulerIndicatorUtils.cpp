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

#include <n4d.hpp>
#include <variant.hpp>
#include <json.hpp>

#include <sys/types.h>

using namespace edupals;
using namespace std;


BellSchedulerIndicatorUtils::BellSchedulerIndicatorUtils(QObject *parent)
    : QObject(parent)
       
{
    client = new n4d::Client("https://localhost",9779);
    variant::Variant bell_info =variant::Variant::create_array(0);

  
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
    QFile file("/home/lliurex/Escritorio/bellscheduler-token");
    file.open(QIODevice::ReadOnly);
    QString s;
    QTextStream s1(&file);
 
    QString line;
    QStringList bells_id;
    while (s1.readLineInto(&line)) {
        bells_id.push_back(line);
    } 
    file.close();
    qDebug()<<"Lista de bells_id" << bells_id;
    
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


