//
//  CBUUIDs.swift
//  Basic Chat MVC
//
//  Created by Trevor Beaton on 2/3/21.
//

import Foundation
import CoreBluetooth

struct CBUUIDs{
    static let BLE_Service_UUID = CBUUID(string: "4587B400-28DF-4DA5-B617-BC2B58CE7930")
    static let BLE_Characteristic_Command_UUID = CBUUID(string: "4587B401-28DF-4DA5-B617-BC2B58CE7930")
    static let BLE_Characteristic_State_UUID = CBUUID(string: "4587B402-28DF-4DA5-B617-BC2B58CE7930")// (Property = Read/Notify)
    static let BLE_Characteristic_TrackingSpeed_UUID = CBUUID(string: "4587B403-28DF-4DA5-B617-BC2B58CE7930")
}

struct TRACKING_COMMAND {
    static let COMMAND_PREFIX = "CMD:"
    static let COMMAND_SPEED_PREFIX = COMMAND_PREFIX + "SPEED:"
    static let REWIND_COMPLETED = COMMAND_PREFIX + "COMPLETED_REWIND"

}
