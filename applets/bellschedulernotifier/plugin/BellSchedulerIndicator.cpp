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
#include "BellSchedulerIndicator.h"
#include "BellSchedulerIndicatorUtils.h"

#include <KLocalizedString>
#include <KFormat>
#include <KNotification>
#include <KRun>
#include <QTimer>
#include <QStandardPaths>
#include <QDebug>
#include <QFile>
#include <QThread>
#include <QtCore/QStringList>
#include <QJsonObject>

#include <variant.hpp>
#include <json.hpp>

using namespace edupals;
using namespace std;



BellSchedulerIndicator::BellSchedulerIndicator(QObject *parent)
    : QObject(parent)
    , m_timer(new QTimer(this))
    , m_timer_run(new QTimer(this))
    , m_utils(new BellSchedulerIndicatorUtils(this))
    
{
    

    TARGET_FILE.setFileName("/tmp/.BellScheduler/bellscheduler-token");
   	
   	cout << "#######BELL INFO######" << endl;
    cout << m_utils->bellsInfo << endl;

    connect(m_timer, &QTimer::timeout, this, &BellSchedulerIndicator::worker);
    m_timer->start(5000);
    worker();
}    


void BellSchedulerIndicator::worker(){


    if (!is_working){
        if (BellSchedulerIndicator::TARGET_FILE.exists() ) {
            getBellInfo();
            isAlive();
            
        }
    }        

     
}    

void BellSchedulerIndicator::showNotification(QString notType,int index){

	qDebug()<<"LANZANDO NOTIFICACION"<<index;
	qDebug()<<"INFORMACION DE LAS ALARMAS";
    cout << m_utils->bellsInfo << endl ; 

	QString hour=QString::fromStdString(m_utils->bellsInfo[index]["hour"]);
	qDebug()<<"################ 1";
	QString bell=QString::fromStdString(m_utils->bellsInfo[index]["name"]);
	qDebug()<<"################ 2";
	QString duration_label=i18n("Duration: ");
	qDebug()<<"################ 3";
	QString duration="";

	if (m_utils->bellsInfo[index]["duration"].get_int32()==0){
		qDebug()<<"################ 4";
		duration=i18n("Full reproduction");
	}else{
		qDebug()<<"################ 5";
		QString label=i18n(" seconds");
		QString s = QString::number(m_utils->bellsInfo[index]["duration"].get_int32());
		qDebug()<<"################ 6";
		duration=s+label;
	}
	qDebug()<<"################ 7";

	if (notType=="start"){
		QString title=i18n("Playing the bell:");
		QString subtooltip=title+"\n-"+hour+" "+bell+"\n-"+duration_label+duration;
		m_bellPlayingNotification = KNotification::event(QStringLiteral("Run"), subtooltip, {}, "bell-scheduler-indicator", nullptr, KNotification::CloseOnTimeout , QStringLiteral("bellschedulernotifier"));
	    QString name = i18n("Stop now");
	    m_bellPlayingNotification->setDefaultAction(name);
	    m_bellPlayingNotification->setActions({name});
	    connect(m_bellPlayingNotification, QOverload<unsigned int>::of(&KNotification::activated), this, &BellSchedulerIndicator::stopBell);
	}else{
		qDebug()<<"################ 8";
		QString title=i18n("The bell has ended:");
		QString subtooltip=title+"\n-"+hour+" "+bell+"\n-"+duration_label+duration;
		m_bellPlayingNotification = KNotification::event(QStringLiteral("Run"), subtooltip, {}, "bell-scheduler-indicator", nullptr, KNotification::CloseOnTimeout , QStringLiteral("bellschedulernotifier"));
		qDebug()<<"################ 9";
	}    
	qDebug()<<"################ 10";

}

void BellSchedulerIndicator::getBellInfo(){

	qDebug()<<"BELLS ID"<<m_utils->bellsId;
    is_working=true;
    m_utils->getBellInfo();

    qDebug()<<"NOTIFICACIONES"<<bellsnotification;

    for (int i=0;i<m_utils->bellsInfo.count();i++){
    	if (!bellsnotification.contains(QString::fromStdString(m_utils->bellsInfo[i]["bellId"]))){
    		bellsnotification.push_back(QString::fromStdString(m_utils->bellsInfo[i]["bellId"]));
 	  		showNotification("start",i);
 	  	}	
    }
   
}

void BellSchedulerIndicator::isAlive(){

	qDebug()<<"Esta vivo";
	bellToken=false;
	changeTryIconState(0);
	connect(m_timer_run, &QTimer::timeout, this, &BellSchedulerIndicator::checkStatus);
    m_timer_run->start(10000);
    checkStatus();


}

bool BellSchedulerIndicator::areBellsLive(){

	bool bellsLive=false;

	QStringList removeBells=m_utils->areBellsLive();
	variant::Variant tmpList=variant::Variant::create_array(0);
	if (removeBells.size()>0){
		bellsLive=true;
		for (int i=0;i<m_utils->bellsInfo.count();i++){
			if (removeBells.contains(QString::fromStdString(m_utils->bellsInfo[i]["bellId"]))){
				showNotification("end",i);

			}else{
				tmpList.append(m_utils->bellsInfo[i]);

			}
		}
		qDebug()<<"################ 11";
		m_utils->bellsInfo=tmpList;
	}

	return bellsLive;


}



void BellSchedulerIndicator::checkStatus(){

	qDebug()<<"Checheando";
	if (BellSchedulerIndicator::TARGET_FILE.exists() ) { 
		if (!bellToken){
			bellToken=true;
			
		}
	}else{
		if (areBellsLive()){
			bellToken=true;
		}else{
			bellToken=false;
			//m_timer_run->stop();
			//changeTryIconState(1);
		}	

	}	

	if (bellToken){
		if (BellSchedulerIndicator::TARGET_FILE.exists()){ 
			if (m_utils->isTokenUpdated()){
				getBellInfo();	
			}
		}	
		qDebug()<<"Asociando token";
		m_utils->linkBellPid();

	}else{
		for (int i=0;i<m_utils->bellsInfo.count();i++){
			showNotification("end",i);
		}	
		m_utils->bellsInfo=variant::Variant::create_array(0);			
		m_timer_run->stop();
		changeTryIconState(1);
		is_working=false;
		QStringList emptyList;
		m_utils->bellsId=emptyList;
		bellsnotification=emptyList;
	}
	
}


BellSchedulerIndicator::TrayStatus BellSchedulerIndicator::status() const
{
    return m_status;
}



void BellSchedulerIndicator::changeTryIconState(int state){

	qDebug()<<"Cambio icono";
    const QString tooltip(i18n("Bell-Scheduler"));
    if (state==0){
    	qDebug()<<"Activando icono";
        setStatus(ActiveStatus);
        const QString subtooltip(i18n("Playing the scheduled bell"));
        setToolTip(tooltip);
        setSubToolTip(subtooltip);
        setIconName("bellschedulernotifier");
        /*
        m_bellPlayingNotification = KNotification::event(QStringLiteral("Run"), subtooltip, {}, "bell-scheduler-indicator", nullptr, KNotification::CloseOnTimeout , QStringLiteral("bellschedulernotifier"));
        const QString name = i18n("Stop now");
        m_bellPlayingNotification->setDefaultAction(name);
        m_bellPlayingNotification->setActions({name});
        connect(m_bellPlayingNotification, QOverload<unsigned int>::of(&KNotification::activated), this, &BellSchedulerIndicator::stopBell);
		*/

    }else{
        setStatus(PassiveStatus);
    }
    


}

void BellSchedulerIndicator::stopBell(){

    m_utils->stopBell();
}


void BellSchedulerIndicator::setStatus(BellSchedulerIndicator::TrayStatus status)
{
    
	qDebug()<<"m_status"<<m_status;
	qDebug()<<"status"<<status;

    if (m_status != status) {
    	qDebug()<<"Cambiando icono";
        m_status = status;
        emit statusChanged();
    }
}

QString BellSchedulerIndicator::iconName() const
{
    return m_iconName;
}

void BellSchedulerIndicator::setIconName(const QString &name)
{
    if (m_iconName != name) {
        m_iconName = name;
        emit iconNameChanged();
    }
}

QString BellSchedulerIndicator::toolTip() const
{
    return m_toolTip;
}

void BellSchedulerIndicator::setToolTip(const QString &toolTip)
{
    if (m_toolTip != toolTip) {
        m_toolTip = toolTip;
        emit toolTipChanged();
    }
}

QString BellSchedulerIndicator::subToolTip() const
{
    return m_subToolTip;
}

void BellSchedulerIndicator::setSubToolTip(const QString &subToolTip)
{
    if (m_subToolTip != subToolTip) {
        m_subToolTip = subToolTip;
        emit subToolTipChanged();
    }
}