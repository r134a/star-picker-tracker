//
//  ViewController.swift
//  tracker
//
//  Created by 김병우 on 2022/05/22.
//

import UIKit
import CoreBluetooth

class ViewController: UIViewController {
    
    private var centralManager: CBCentralManager!
    private var connectedService: CBService!
    private var connectedPeripheral: CBPeripheral!
    private var commandCharacteristic: CBCharacteristic!
    private var stateCharacteristic: CBCharacteristic!
    private var trackingSpeedCharacteristic: CBCharacteristic!
    private var peripheralArray: [CBPeripheral] = []
    private var rssiArray = [NSNumber]()
    private var timer = Timer()
    
    @IBOutlet weak var startButton: UIButton!
    @IBOutlet weak var rewindButton: UIButton!
    @IBOutlet weak var backwardButton: UIButton!
    @IBOutlet weak var trackingSpeedApplyButton: UIButton!
    @IBOutlet weak var consoleView: UITextView!
    @IBOutlet weak var trackingSpeedTextField: UITextField!
    
    
    override func viewDidLoad() {
        super.viewDidLoad()
        trackingSpeedTextField.keyboardType = UIKeyboardType.decimalPad
        centralManager = CBCentralManager(delegate: self, queue: nil)
        
        // keyboard show & hide
        self.hideKeyboardWhenTappedAround()
    }
    
    var isCommandButtonClicked = false
    
    @IBAction func startAction(_ sender: Any) {
        isCommandButtonClicked = !isCommandButtonClicked
        
        if( isCommandButtonClicked ) {
            rewindButton.isEnabled = false
            backwardButton.isEnabled = false
            writeStateMessage(message: "Start tracking the stars!!.", line: true)
            startButton.setTitle("Stop", for: .normal)
            writeToCommand(data: 1)
        }
        else {
            writeToCommand(data: 2)
            rewindButton.isEnabled = true
            backwardButton.isEnabled = true
            writeStateMessage(message: "Stop tracking.")
            startButton.setTitle("Start", for: .normal)
        }
    }
    
    @IBAction func rewindAction(_ sender: Any) {
        isCommandButtonClicked = !isCommandButtonClicked
        
        if( isCommandButtonClicked ) {
            startButton.isEnabled = false
            backwardButton.isEnabled = false
            writeStateMessage(message: "Start a rewind.", line: true)
            rewindButton.setTitle("Stop", for: .normal)
            writeToCommand(data: 3)
        }
        else {
            writeToCommand(data: 2)
            startButton.isEnabled = true
            backwardButton.isEnabled = true
            writeStateMessage(message: "Stop rewind.")
            rewindButton.setTitle("Rewind", for: .normal)
        }
    }
    
    @IBAction func backwardAction(_ sender: Any) {
        isCommandButtonClicked = !isCommandButtonClicked
        
        if( isCommandButtonClicked ) {
            startButton.isEnabled = false
            rewindButton.isEnabled = false
            writeStateMessage(message: "Start backward.", line: true)
            backwardButton.setTitle("Stop", for: .normal)
            writeToCommand(data: 4)
        }
        else {
            writeToCommand(data: 2)
            startButton.isEnabled = true
            rewindButton.isEnabled = true
            writeStateMessage(message: "Stop backward.")
            backwardButton.setTitle("Backward", for: .normal)
        }
    }
    
    @IBAction func trackingSpeedApplyAction(_ sender: Any) {
        let trackingSpeed = Double(trackingSpeedTextField.text!) ?? 0
        print( trackingSpeed )
        writeToTrackingSpeed(data: trackingSpeed)
    }
    
    
    func writeStateMessage(message: String) -> Void {
        writeStateMessage(message: message, line: false)
    }
    
    
    func writeStateMessage(message: String, line: Bool) -> Void {
        if( message.hasPrefix( TRACKING_COMMAND.COMMAND_PREFIX ) ) {
            return
        }
        
        if(line) {consoleView.text.append("---------------------------------------\n")}
        consoleView.scrollRangeToVisible(NSRange(location: consoleView.text.count - 1, length: 0))
        consoleView.text.append(message + "\n")
        consoleView.scrollRangeToVisible(NSRange(location: consoleView.text.count - 1, length: 0))
        if(line) {consoleView.text.append("---------------------------------------\n")}
        consoleView.scrollRangeToVisible(NSRange(location: consoleView.text.count - 1, length: 0))
        
    }
    
    func startScanning() -> Void {
        // Remove prior data
        peripheralArray.removeAll()
        rssiArray.removeAll()
        // Start Scanning
        centralManager?.scanForPeripherals(withServices: [CBUUIDs.BLE_Service_UUID])
        
        writeStateMessage(message: "Start Scanning ... ")
        trackingSpeedApplyButton.isEnabled = false
        Timer.scheduledTimer(withTimeInterval: 15, repeats: false) {_ in
            self.stopScanning()
        }
    }
    
    func checkCommandOfCompletedRewind(message: String) -> Void {
        if( TRACKING_COMMAND.REWIND_COMPLETED == message) {
            rewindButton.setTitle("Rewind", for: .normal)
            startButton.isEnabled = true
            backwardButton.isEnabled = true
            isCommandButtonClicked = !isCommandButtonClicked
            writeStateMessage(message: "Rewind completed.", line: true)
            
        }
    }
    
    func checkCommandOfTrackingSpeed(message: String) -> Void {
        if( message.hasPrefix(TRACKING_COMMAND.COMMAND_SPEED_PREFIX) ) {
            trackingSpeedTextField.text = message.substring(from: TRACKING_COMMAND.COMMAND_SPEED_PREFIX.count)
            trackingSpeedApplyButton.isEnabled = true
        }
    }
    
    
    func disconnectFromDevice() -> Void {
        if connectedPeripheral != nil {
            centralManager?.cancelPeripheralConnection(connectedPeripheral!)
        }
    }
    
    func removeArrayData() -> Void {
        centralManager.cancelPeripheralConnection(connectedPeripheral)
        rssiArray.removeAll()
        peripheralArray.removeAll()
    }
    func stopTimer() -> Void {
        // Stops Timer
        self.timer.invalidate()
    }
    
    func stopScanning() -> Void {
        writeStateMessage(message: "Stop Scanning ...")
        centralManager?.stopScan()
    }
    
    // Write Command functions
    func writeToCommand(data: Int){
        let value = withUnsafeBytes(of: data) { Data($0) }
        connectedPeripheral.writeValue(value, for: commandCharacteristic, type: CBCharacteristicWriteType.withResponse)
    }
    
    
    // Write TrackingSpeed functions
    func writeToTrackingSpeed(data: Double){
        let value = withUnsafeBytes(of: data) { Data($0) }
        connectedPeripheral.writeValue(value, for: trackingSpeedCharacteristic, type: CBCharacteristicWriteType.withResponse)
    }
    
    
}

// Put this piece of code anywhere you like
extension UIViewController {
    func hideKeyboardWhenTappedAround() {
        let tap = UITapGestureRecognizer(target: self, action: #selector(UIViewController.dismissKeyboard))
        tap.cancelsTouchesInView = false
        view.addGestureRecognizer(tap)
    }
    
    @objc func dismissKeyboard() {
        view.endEditing(true)
    }
}

extension String {
    func index(from: Int) -> Index {
        return self.index(startIndex, offsetBy: from)
    }

    func substring(from: Int) -> String {
        let fromIndex = index(from: from)
        return String(self[fromIndex...])
    }

    func substring(to: Int) -> String {
        let toIndex = index(from: to)
        return String(self[..<toIndex])
    }

    func substring(with r: Range<Int>) -> String {
        let startIndex = index(from: r.lowerBound)
        let endIndex = index(from: r.upperBound)
        return String(self[startIndex..<endIndex])
    }
}

extension Array {
    subscript (safe index: Int) -> Element? {
        // iOS 9 or later
        return indices ~= index ? self[index] : nil
        // iOS 8 or earlier
        // return startIndex <= index && index < endIndex ? self[index] : nil
        // return 0 <= index && index < self.count ? self[index] : nil
    }
}

extension ViewController: CBCentralManagerDelegate {
    func centralManagerDidUpdateState(_ central: CBCentralManager) {
        
        switch central.state {
        case .poweredOff:
            print("Is Powered Off.")
            
            let alertVC = UIAlertController(title: "Bluetooth Required", message: "Check your Bluetooth Settings", preferredStyle: UIAlertController.Style.alert)
            
            let action = UIAlertAction(title: "Ok", style: UIAlertAction.Style.default, handler: { (action: UIAlertAction) -> Void in
                self.dismiss(animated: true, completion: nil)
            })
            
            alertVC.addAction(action)
            
            self.present(alertVC, animated: true, completion: nil)
            
        case .poweredOn:
            print("Is Powered On.")
            startScanning()
        case .unsupported:
            print("Is Unsupported.")
        case .unauthorized:
            print("Is Unauthorized.")
        case .unknown:
            print("Unknown")
        case .resetting:
            print("Resetting")
        @unknown default:
            print("Error")
        }
    }
    
    // MARK: - Discover
    func centralManager(_ central: CBCentralManager, didDiscover peripheral: CBPeripheral, advertisementData: [String : Any], rssi RSSI: NSNumber) {
        if peripheralArray.contains(peripheral) {
            print("Duplicate Found.")
        } else {
            peripheralArray.append(peripheral)
            rssiArray.append(RSSI)
        }
        
        connectedPeripheral = peripheral
        connectedPeripheral.delegate = self
        
        print("Peripheral Discovered: \(peripheral)")
        writeStateMessage(message: "Discovered: \(peripheral)")
        
        centralManager?.connect(peripheral, options: nil)
        writeStateMessage(message: "Connected ...")
    }
    
    // MARK: - Connect
    func centralManager(_ central: CBCentralManager, didConnect peripheral: CBPeripheral) {
        stopScanning()
        connectedPeripheral.discoverServices([CBUUIDs.BLE_Service_UUID])
    }
}

extension ViewController: CBPeripheralDelegate {
    
    func peripheral(_ peripheral: CBPeripheral, didDiscoverServices error: Error?) {
        
        guard let services = peripheral.services else { return }
        for service in services {
            peripheral.discoverCharacteristics(nil, for: service)
        }
        connectedService = services[0]
    }
    
    func peripheral(_ peripheral: CBPeripheral, didDiscoverCharacteristicsFor service: CBService, error: Error?) {
        guard let characteristics = service.characteristics else {
            return
        }
        
        print("Found \(characteristics.count) characteristics.")
        writeStateMessage(message: "Found \(characteristics.count) characteristics")
        
        for characteristic in characteristics {
            if characteristic.uuid.isEqual(CBUUIDs.BLE_Characteristic_State_UUID)  {
                stateCharacteristic = characteristic
                
                peripheral.setNotifyValue(true, for: stateCharacteristic!)
                peripheral.readValue(for: characteristic)
                
                writeStateMessage(message: "CMD-UUID: \(stateCharacteristic.uuid)")
            }
            
            if characteristic.uuid.isEqual(CBUUIDs.BLE_Characteristic_Command_UUID){
                commandCharacteristic = characteristic
                writeStateMessage(message: "STATE-UUID: \(commandCharacteristic.uuid)")
            }
            
            if characteristic.uuid.isEqual(CBUUIDs.BLE_Characteristic_TrackingSpeed_UUID){
                trackingSpeedCharacteristic = characteristic
                writeStateMessage(message: "SPEED-UUID: \(trackingSpeedCharacteristic.uuid)")
            }
        }
    }
    
    
    // Notify
    func peripheral(_ peripheral: CBPeripheral, didUpdateValueFor characteristic: CBCharacteristic, error: Error?) {
        if( characteristic == stateCharacteristic ) {
            let value = characteristic.value
            let valueString = NSString(data: value!, encoding: String.Encoding.utf8.rawValue)
            let message = valueString! as String
            writeStateMessage(message: "\(message)")
            checkCommandOfCompletedRewind(message: message)
            checkCommandOfTrackingSpeed(message: message)
        }
    }
    
    func peripheral(_ peripheral: CBPeripheral, didReadRSSI RSSI: NSNumber, error: Error?) {
        peripheral.readRSSI()
    }
    
    func peripheral(_ peripheral: CBPeripheral, didWriteValueFor characteristic: CBCharacteristic, error: Error?) {
        guard error == nil else {
            print("Error discovering services: error")
            writeStateMessage(message: "Error discovering services: error")
            return
        }
    }
    
    
    func peripheral(_ peripheral: CBPeripheral, didUpdateNotificationStateFor characteristic: CBCharacteristic, error: Error?) {
        print("*******************************************************")
        print("Function: \(#function),Line: \(#line)")
        if (error != nil) {
            print("Error changing notification state:\(String(describing: error?.localizedDescription))")
            
        } else {
            print("Characteristic's value subscribed")
        }
        
        if (characteristic.isNotifying) {
            print ("Subscribed. Notification has begun for: \(characteristic.uuid)")
        }
    }
    
}

extension ViewController: CBPeripheralManagerDelegate {
    
    func peripheralManagerDidUpdateState(_ peripheral: CBPeripheralManager) {
        switch peripheral.state {
        case .poweredOn:
            print("Peripheral Is Powered On.")
        case .unsupported:
            print("Peripheral Is Unsupported.")
        case .unauthorized:
            print("Peripheral Is Unauthorized.")
        case .unknown:
            print("Peripheral Unknown")
        case .resetting:
            print("Peripheral Resetting")
        case .poweredOff:
            print("Peripheral Is Powered Off.")
        @unknown default:
            print("Error")
        }
    }
    
    
    //Check when someone subscribe to our characteristic, start sending the data
    func peripheralManager(_ peripheral: CBPeripheralManager, central: CBCentral, didSubscribeTo characteristic: CBCharacteristic) {
        print("Device subscribe to characteristic")
    }
    
}
