/*
 * Copyright (c) 2009, Swedish Institute of Computer Science.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the Institute nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE INSTITUTE AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE INSTITUTE OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

import gnu.io.CommPortIdentifier;
import gnu.io.SerialPortEvent;
import gnu.io.SerialPortEventListener;

import java.awt.BorderLayout;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.io.File;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.util.ArrayList;
import java.util.Collection;
import java.util.TooManyListenersException;

import javax.swing.JLabel;
import javax.swing.JPanel;

import org.apache.log4j.Logger;
import org.jdom.Element;

import se.sics.cooja.ClassDescription;
import se.sics.cooja.Mote;
import se.sics.cooja.dialogs.CompileContiki;
import se.sics.cooja.dialogs.MessageList; 
import se.sics.cooja.dialogs.SerialUI;

@ClassDescription("Real Serial Port")
public class RealSerialPortMoteInterface extends SerialUI {
	private static Logger logger = Logger.getLogger(RealSerialPortMoteInterface.class);

	private RealSerialPortMoteType.RealSerialPortMote mote;

	private String configuredComPort = "";
	private String comPort = null;

	private gnu.io.SerialPort serialPort = null;
	private InputStream serialPortInputStream = null;
	private OutputStream serialPortOutputStream = null;

	private JLabel statusLabel = new JLabel("");

	public RealSerialPortMoteInterface(Mote mote) {
		this.mote = (RealSerialPortMoteType.RealSerialPortMote) mote;
		disconnect();
	}

	public void setComPort(String comPort) {
		configuredComPort = comPort;

		if (this.comPort != null && this.comPort.equals(comPort)) {
			/* Nothing to do - we are already connected! */
		}

		/* Disconnect */
		disconnect();

		/* Locate new comport */
		if (comPort == null || comPort.equals("")) {
			return;
		}
		CommPortIdentifier comPortID = MoteProgrammerPlugin.getCOMPortIdentifier(comPort);
		if (comPortID == null) {
			MoteProgrammerPlugin.scanAllCOMPorts();
		}
		comPortID = MoteProgrammerPlugin.getCOMPortIdentifier(comPort);
		if (comPortID == null) {
			logger.fatal("Failed locating serial port: " + comPort);
			statusLabel.setText("Failed locating serial port: " + comPort);
			String msg = "[!" + comPort + "]";
			for (byte b: msg.getBytes()) {
				dataReceived(b);
			}
			dataReceived('\n');
			return;
		}

		/* Connect to new comport */
		try {
			serialPort = (gnu.io.SerialPort) comPortID.open("SimpleReadApp", 2000);
			serialPort.notifyOnOutputEmpty(false);
			serialPort.notifyOnDataAvailable(true);
			serialPort.setSerialPortParams(
					115200,
					gnu.io.SerialPort.DATABITS_8,
					gnu.io.SerialPort.STOPBITS_1,
					gnu.io.SerialPort.PARITY_NONE
					);
			serialPort.setFlowControlMode(gnu.io.SerialPort.FLOWCONTROL_XONXOFF_OUT);
		} catch (Exception e) {
			logger.fatal("Failed connecting to comport: " + comPort + ": " + e.getMessage());
			statusLabel.setText("Failed connecting to comport: " + comPort + ": " + e.getMessage());
			String msg = "[!" + comPort + "]";
			for (byte b: msg.getBytes()) {
				dataReceived(b);
			}
			dataReceived('\n');
			return;
		}

		/* Get I/O streams */
		try {
			serialPortInputStream = serialPort.getInputStream();
			serialPortOutputStream = serialPort.getOutputStream();
			addSerialPortLineListener(serialPort, serialPortInputStream, new ActionListener() {
				public void actionPerformed(final ActionEvent e) {
					/* Signal new message in simulation thread */
					mote.getSimulation().invokeSimulationThread(new Runnable() {
						public void run() {
							for (byte b: e.getActionCommand().getBytes()) {
								dataReceived(b);
							}
							dataReceived('\n');
						}
					});
				}
			});
		} catch (IOException e) {
			logger.fatal("Failed connecting to serial port: " + comPort + ": " + e.getMessage());
			statusLabel.setText("Failed connecting to serial port: " + comPort + ": " + e.getMessage());
			String msg = "[!" + comPort + "]";
			for (byte b: msg.getBytes()) {
				dataReceived(b);
			}
			dataReceived('\n');
			return;
		}

		/* Done! */
		this.comPort = comPort;
		statusLabel.setText("connected: " + comPort);
		String msg = "[connected: " + comPort + "]";
		for (byte b: msg.getBytes()) {
			dataReceived(b);
		}
		dataReceived('\n');
	}

	private void disconnect() {
		statusLabel.setText("disconnected");
		if (comPort == null) {
			return;
		}

		if (serialPortInputStream != null) {
			try {
				serialPortInputStream.close();
			} catch (Exception e) { }
		}
		serialPortInputStream = null;

		if (serialPortOutputStream != null) {
			try {
				serialPortOutputStream.close();
			} catch (Exception e) { }
		}
		serialPortOutputStream = null;

		if (serialPort != null) {
			try {
				serialPort.close();
			} catch (Exception e) { }
		}
		serialPort = null;

		logger.info("Disconnected from comport: " + comPort);
		comPort = null;
	}

	public boolean isConnected() {
		if (comPort == null) {
			return false;
		}
		if (serialPort == null) {
			return false;
		}
		return true;
	}

	public void removed() {
		super.removed();
		disconnect();
	}

	public String getConfiguredComPort() {
		return configuredComPort;
	}
	public String getActualComPort() {
		if (comPort == null) {
			return "";
		}
		return comPort;
	}

	public JPanel getInterfaceVisualizer() {
		JPanel panel = super.getInterfaceVisualizer();
		JPanel wrapper = new JPanel(new BorderLayout());
		wrapper.add(BorderLayout.CENTER, panel);
		wrapper.add(BorderLayout.NORTH, statusLabel);
		return wrapper; 
	}

	public void releaseInterfaceVisualizer(JPanel panel) {
		panel.remove(statusLabel);
		super.releaseInterfaceVisualizer((JPanel)panel.getComponent(0));
	}

	public Collection<Element> getConfigXML() {
		Collection<Element> config = new ArrayList<Element>();
		Element element = new Element("comport");
		element.setAttribute("comport", getConfiguredComPort());
		config.add(element);
		return config;
	}
	public void setConfigXML(Collection<Element> configXML, boolean visAvailable) {
		for (Element element: configXML) {
			String name = element.getName();
			if (name.equals("comport")) {
				String comPort = (String) element.getAttribute("comport").getValue();
				setComPort(comPort);
			}
		}
	}

	private static void addSerialPortLineListener(
			final gnu.io.SerialPort serialPort,
			final InputStream inputStream,
			final ActionListener listener) {
		try {
			serialPort.addEventListener(new SerialPortEventListener() {
				private StringBuilder sb = new StringBuilder();

				public void serialEvent(SerialPortEvent event) {
					if (event.getEventType() == SerialPortEvent.DATA_AVAILABLE) {
						try {
							while (inputStream.available() > 0) {
								char c = (char) inputStream.read();
								if ((int) c == 10) {
									if (sb.length() == 0) {
										logger.warn("Length is zero");
									}
									listener.actionPerformed(new ActionEvent(serialPort, 0, sb.toString()));
									sb = new StringBuilder();
								} else {
									sb.append(c);
								}
							}
						} catch (IOException e) {
							logger.fatal(e);
						}
					}
				}
			});
		} catch (TooManyListenersException e) {
			logger.fatal("Too many listeners", e);
		}
	}

	public Mote getMote() {
		return mote;
	}

	public void writeArray(byte[] s) {
		if (!isConnected()) {
			logger.warn("Not connected");
			return;
		}
		try {
			serialPortOutputStream.write(s);
		} catch (IOException e) {
			logger.fatal("Error: " + e.getMessage());
			disconnect();
		}
	}
	public void writeByte(byte b) {
		if (!isConnected()) {
			logger.warn("Not connected");
			return;
		}
		try {
			serialPortOutputStream.write(b);
		} catch (IOException e) {
			logger.fatal("Error: " + e.getMessage());
			disconnect();
		}
	}
	public void writeString(String s) {
		if (!isConnected()) {
			logger.warn("Not connected");
			return;
		}
		try {
			serialPortOutputStream.write(s.getBytes());
			serialPortOutputStream.write((byte) '\n');
		} catch (IOException e) {
			logger.fatal("Error: " + e.getMessage());
			disconnect();
		}
	}

}