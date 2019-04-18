package com.fraioveio.wirewrappercontroller;

import com.fraioveio.wirewrappercontroller.gui.ControllerFrame;
import javax.swing.UIManager;

public class WireWrapperController {
    public static void main(String[] args) {
        try {
            UIManager.setLookAndFeel(UIManager.getSystemLookAndFeelClassName());
        } catch (Exception ex) {}
        ControllerFrame f = new ControllerFrame();
        f.setVisible(true);
    }
}