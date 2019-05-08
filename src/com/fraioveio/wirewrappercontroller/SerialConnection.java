package com.fraioveio.wirewrappercontroller;

import gnu.io.CommPort;
import gnu.io.CommPortIdentifier;
import gnu.io.NoSuchPortException;
import gnu.io.PortInUseException;
import gnu.io.SerialPort;
import gnu.io.UnsupportedCommOperationException;
import java.io.DataInputStream;
import java.io.DataOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.util.Enumeration;
import java.util.logging.Level;
import java.util.logging.Logger;
import javax.swing.JOptionPane;

public class SerialConnection {
    private static volatile String portlist;
    private static DataInputStream in;
    private static DataOutputStream out;
    private static CommPort commPort;
    
    public static String getPortList() {
        while(portlist == null) {
            try {
                Thread.sleep(10);
            } catch (InterruptedException ex) {}
        }
        return portlist;
    }
    
    public static void refreshPortList() {
        Enumeration<CommPortIdentifier> portEnum = CommPortIdentifier.getPortIdentifiers();
        String lp = "";
        while (portEnum.hasMoreElements()) {
            CommPortIdentifier portIdentifier = portEnum.nextElement();
            lp += portIdentifier.getName() + "\n";
        }
        
        portlist = lp;
    }
    
    public static boolean test() {
        try {
            out.write(66);  // Test cmd
            out.write(12);
            while(in.available() == 0);
            int r = in.read();
            return r == 12;
        } catch (IOException ex) {
            return false;
        }
    }
    
    public static boolean connect(String portname) {
        try {
            CommPortIdentifier portIdentifier = CommPortIdentifier.getPortIdentifier(portname);
            if (portIdentifier.isCurrentlyOwned()) {
                JOptionPane.showMessageDialog(null, "La porta è in uso da un altro processo", "Errore", JOptionPane.ERROR_MESSAGE);
                return false;
            } else {
                commPort = portIdentifier.open("WireWrapper", 2000);
                
                if (commPort instanceof SerialPort) {
                    SerialPort serialPort = (SerialPort) commPort;
                    serialPort.setSerialPortParams(57600, SerialPort.DATABITS_8, SerialPort.STOPBITS_1, SerialPort.PARITY_NONE);
                    
                    in = new DataInputStream(serialPort.getInputStream());
                    out = new DataOutputStream(serialPort.getOutputStream());
                    
                } else {
                    JOptionPane.showMessageDialog(null, "Impossibile gestire porte non seriali", "Errore", JOptionPane.ERROR_MESSAGE);
                    return false;
                }
            }            
        } catch (NoSuchPortException noSuchPortException) {
            JOptionPane.showMessageDialog(null, "La porta non esiste", "Errore", JOptionPane.ERROR_MESSAGE);
            noSuchPortException.printStackTrace();
            return false;
        } catch (PortInUseException portInUseException) {
            JOptionPane.showMessageDialog(null, "La porta è in uso da un altro processo", "Errore", JOptionPane.ERROR_MESSAGE);
            return false;
        } catch (IOException ex) {
            JOptionPane.showMessageDialog(null, ex.toString(), "Errore", JOptionPane.ERROR_MESSAGE);
            return false;
        } catch (UnsupportedCommOperationException ex) {
            JOptionPane.showMessageDialog(null, ex.toString(), "Errore", JOptionPane.ERROR_MESSAGE);
        }
        
        return true;
    }
    
    public static void close() {
        try {
            in.close();
            out.close();
            commPort.close();
        } catch (IOException | NullPointerException ex) {}
    }
}
