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

#include <variant.hpp>

using namespace edupals;
using namespace std;



BellSchedulerIndicator::BellSchedulerIndicator(QObject *parent)
    : QObject(parent)
    , m_timer(new QTimer(this))
    ,m_utils(new BellSchedulerIndicatorUtils(this))
    
{
    

    TARGET_FILE.setFileName("/home/lliurex/Escritorio/bellscheduler-token");
   

    connect(m_timer, &QTimer::timeout, this, &BellSchedulerIndicator::worker);
    m_timer->start(5000);
    worker();
}    


void BellSchedulerIndicator::worker(){


    if (!is_working){
        if (BellSchedulerIndicator::TARGET_FILE.exists() ) {
            getBellInfo();
            
        }
    }        

     
}    

void BellSchedulerIndicator::getBellInfo(){

    is_working=true;
    variant::Variant bellsList = m_utils->readToken();
    cout << "#############" << endl;
    cout << bellsList << endl;

}

void BellSchedulerIndicator::stopBell(){

    qDebug()<<"Parando alarma";
}



BellSchedulerIndicator::TrayStatus BellSchedulerIndicator::status() const
{
    return m_status;
}



void BellSchedulerIndicator::setStatus(BellSchedulerIndicator::TrayStatus status)
{
    if (m_status != status) {
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