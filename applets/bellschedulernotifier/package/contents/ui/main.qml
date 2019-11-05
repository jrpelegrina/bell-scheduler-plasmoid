import QtQuick 2.2
import QtQuick.Layouts 1.1
import QtQml.Models 2.3
import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.plasma.plasmoid 2.0
import org.kde.plasma.components 2.0 as Components
import org.kde.plasma.extras 2.0 as PlasmaExtras

import org.kde.plasma.private.bellschedulernotifier 1.0
// Item - the most basic plasmoid component, an empty container.
Item {

	id:bellschedulerApplet
	
	BelllSchedulerIndicator{
        id:bellSchedulerIndicator

    }


    Plasmoid.status: {
        /* Warn! Enum types are accesed through ClassName not ObjectName */
        switch (bellSchedulerIndicator.status){
            case BellSchedulerIndicator.NeedsAttentionStatus: 
                return PlasmaCore.Types.NeedsAttentionStatus
            case BellSchedulerIndicator.PassiveStatus:
                return PlasmaCore.Types.PassiveStatus
           
        }
        return  PlasmaCore.Types.PassiveStatus
        
    }

    Plasmoid.switchWidth: units.gridUnit * 5
    Plasmoid.switchHeight: units.gridUnit * 5

    Plasmoid.icon:bellSchedulerIndicator.iconName
    Plasmoid.toolTipMainText: bellSchedulerIndicator.toolTip
    Plasmoid.toolTipSubText: bellSchedulerIndicator.subToolTip

    Component.onCompleted: {
       plasmoid.removeAction("configure");
       plasmoid.setAction("stop", i18n("Stop the alarm"), "stop"); 
                  
    }

   
	Plasmoid.onActivated: action_stop()
	Plasmoid.preferredRepresentation: Plasmoid.compactRepresentation
    Plasmoid.compactRepresentation: PlasmaCore.IconItem {
        source: plasmoid.icon
        
    }

    Plasmoid.onExpandedChanged: if (Plasmoid.expanded) {
        action_stop()
    }

    
    function action_stop() {
    	
        bellSchedulerIndicator.stop_alarm()
        /*plasmoid.activated()*/

    }

    
 }	