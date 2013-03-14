/*
 * Copyright (c) 2012, Thingsquare, http://www.thingsquare.com/.
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
 *
 * 3. Neither the name of the copyright holder nor the names of its
 *    contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE
 * COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
 * OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */

import gnu.io.CommPortIdentifier;

import java.awt.BorderLayout;
import java.awt.Color;
import java.awt.Window;
import java.io.File;
import java.io.FileNotFoundException;
import java.lang.reflect.Field;
import java.net.URISyntaxException;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.Collection;
import java.util.Enumeration;

import javax.swing.AbstractAction;
import javax.swing.Action;
import javax.swing.Box;
import javax.swing.DefaultCellEditor;
import javax.swing.JButton;
import javax.swing.JComboBox;
import javax.swing.JDialog;
import javax.swing.JOptionPane;
import javax.swing.JProgressBar;
import javax.swing.JScrollPane;
import javax.swing.JTable;
import javax.swing.table.AbstractTableModel;
import javax.swing.table.TableCellEditor;

import org.apache.log4j.Logger;
import org.jdom.Element;

import se.sics.cooja.ClassDescription;
import se.sics.cooja.GUI;
import se.sics.cooja.GUI.RunnableInEDT;
import se.sics.cooja.HasQuickHelp;
import se.sics.cooja.Mote;
import se.sics.cooja.MoteInterface;
import se.sics.cooja.MoteType;
import se.sics.cooja.PluginType;
import se.sics.cooja.SimEventCentral.MoteCountListener;
import se.sics.cooja.Simulation;
import se.sics.cooja.VisPlugin;
import se.sics.cooja.dialogs.CompileContiki;
import se.sics.cooja.dialogs.MessageList;
import se.sics.cooja.dialogs.MessageList.MessageContainer;
import se.sics.cooja.plugins.Visualizer;
import se.sics.cooja.util.ArrayUtils;

/* EXAMPLE USAGE (contiki-sky-main.c) */
//  /* Hardcoded IDs instead of loading from from eeprom/ds2411 */
//  struct preset_id {
//    uint16_t ignore_preset;
//    uint16_t nodeid;
//    uint16_t blabla;
//  };
//  struct preset_id *my_id = (struct preset_id*) "PRESET_VALUES_IDENTIFIER";

//  /* Restore node id if such has been stored in external mem */
//  if (my_id->ignore_preset == 0) {
//    printf("Using preset IDs\n");
//    node_id = my_id->nodeid;
//  }

@ClassDescription("Mote programmer")
@PluginType(PluginType.SIM_PLUGIN)
public class MoteProgrammerPlugin extends VisPlugin implements HasQuickHelp {
	private static Logger logger = Logger.getLogger(MoteProgrammerPlugin.class);
	private static CommPortIdentifier[] allCOMPorts = new CommPortIdentifier[0];

	{
		/* Register visualizer skin */
		Visualizer.registerVisualizerSkin(MoteProgrammerSkin.class);
	}

	private ArrayList<MoteCOMPort> moteComPorts = new ArrayList<MoteCOMPort>();
	private class MoteCOMPort {
		final Mote mote;
		String progPort = null;
		String serialPort = null;
		File firmware = null;
		public MoteCOMPort(Mote mote) {
			this.mote = mote;
		}
	}

	private final static int IDX_MOTE = 0;
	private final static int IDX_PROGPORT = 1;
	private final static int IDX_SERIALPORT = 2;
	private final static int IDX_PROGRAM = 3;
	private final static String[] columns = new String[] {
		"Mote", "Programming port", "Serial port", "Program"
	};
	private JTable moteComPortTable = null;

	private MoteCountListener newMotesListener = new MoteCountListener() {
		public void moteWasAdded(Mote mote) {
			moteComPorts.add(new MoteCOMPort(mote));
			model.fireTableDataChanged();
		}
		public void moteWasRemoved(Mote mote) {
			MoteCOMPort[] arr = moteComPorts.toArray(new MoteCOMPort[0]);
			for (MoteCOMPort motePort: arr) {
				if (motePort.mote == mote) {
					moteComPorts.remove(motePort);
					model.fireTableDataChanged();
					return;
				}
			}
		}
	};

	private Simulation simulation;

	public MoteProgrammerPlugin(Simulation simulation, GUI gui) {
		super("Mote programmer", gui);
		this.simulation = simulation;

		/* Singleton plugin */
		if (simulation.getGUI().getStartedPlugin(this.getClass().getName()) != null) {
			throw new RuntimeException("Only one " + GUI.getDescriptionOf(this) + " is allowed!");
		}

		/* List available COM ports */
		scanAllCOMPorts();
		updateAction.putValue(Action.NAME, "Rescan COM ports (" + allCOMPorts.length + ")");
		updateAction.putValue(Action.SHORT_DESCRIPTION, joinStrings(getAllCOMPorts()));

		/* Populate table with motes */
		simulation.getEventCentral().addMoteCountListener(newMotesListener);
		for (Mote m: simulation.getMotes()) {
			newMotesListener.moteWasAdded(m);
		}

		/* Mote <-> COM ports table */
		final JComboBox<Object> combo = new JComboBox<Object>();
		combo.setEditable(false);
		moteComPortTable = new JTable(model) {
			public TableCellEditor getCellEditor(int row, int column) {
				if (column == IDX_PROGPORT ||
						column == IDX_SERIALPORT) {
					String[] ports = getAllCOMPorts();
					combo.removeAllItems();
					combo.addItem("[no comport]");
					for (String p: ports) {
						combo.addItem(p);
					}
				}
				return super.getCellEditor(row, column);
			}
		};
		moteComPortTable.getColumnModel().getColumn(IDX_PROGPORT).setCellEditor(new DefaultCellEditor(combo));
		moteComPortTable.getColumnModel().getColumn(IDX_SERIALPORT).setCellEditor(new DefaultCellEditor(combo));
		moteComPortTable.getColumnModel().getColumn(IDX_PROGRAM).setPreferredWidth(60); /* XXX */
		moteComPortTable.getColumnModel().getColumn(IDX_PROGRAM).setMaxWidth(60);
		model.fireTableDataChanged();

		/* Control panel */
		Box controlBox = Box.createHorizontalBox();
		controlBox.add(new JButton(updateAction));
		controlBox.add(new JButton(programAction));
		{
			JButton attachButton = new JButton(attachAction);
			attachButton.setForeground(Color.RED);
			controlBox.add(attachButton);
		}

		/* Layout */
		final JScrollPane scrollPane = new JScrollPane(moteComPortTable);
		getContentPane().setLayout(new BorderLayout());
		add(BorderLayout.CENTER, scrollPane);
		add(BorderLayout.SOUTH, controlBox);
		setSize(400, 300);
	}

	public void programMotesNow() {
		programFirmwaresNow(moteComPorts.toArray(new MoteCOMPort[0]));
	}

	public void programFirmwaresNow(final MoteCOMPort[] motes) {
		new Thread() {
			public void run() {
				/* Prepare mote firmwares */
				boolean prepare = simulation.getGUI().getProjectConfig().getBooleanValue(MoteProgrammerPlugin.class, "PREPARE_FIRMWARES", true);
				if (prepare) {
					if (!prepareFirmwares(motes)) {
						return;
					}
				} else {
					for (MoteCOMPort moteComPort: motes) {
						moteComPort.firmware = moteComPort.mote.getType().getContikiFirmwareFile();

						/* XXX FROS TODO Replacing .exp5438 with mist-exp5438! */
						logger.warn("TODO XXX WARNING: Replacing all .exp5438 with .mist-exp5438!");
						String file = moteComPort.firmware.getAbsolutePath();
						moteComPort.firmware = new File(file.replace(".exp5438", ".mist-exp5438"));
					}
				}

				/* Execute programming commands */
				executeProgrammingCommands(motes);
			}
		}.start();
	}

	private static void writeInt(int val, byte[] arr, int offset) {
		byte tmp[] = new byte[2];
		tmp[1] = (byte) ((val & 0xFF00) >> 8);
		tmp[0] = (byte) ((val & 0xFF) >> 0);
		System.arraycopy(tmp, 0, arr, offset, 2);
	}

	private static final String PRESET_ID_STRING = "PRESET_VALUES_IDENTIFIER";
	private boolean prepareFirmwares(MoteCOMPort[] motes) {
		for (MoteCOMPort moteComPort: motes) {
			if (moteComPort.mote == null || moteComPort.serialPort == null || 
					moteComPort.progPort == null) {
				moteComPort.firmware = null;
				continue;
			}

			/* Copy and update mote-unique firmware*/
			byte[] firmware = ArrayUtils.readFromFile(moteComPort.mote.getType().getContikiFirmwareFile());
			File firmwareFile = new File("moteprogrammer_" + moteComPort.mote.getID() + ".firmware");

			/* Look for "PRESET_VALUES" string in memory */
			byte[] presetIdBytes = PRESET_ID_STRING.getBytes();
			int matchingIndex = -1;
			for (int i=0; i < firmware.length-presetIdBytes.length; i++) {
				byte[] cmpArr = Arrays.copyOfRange(firmware, i, i+presetIdBytes.length);
				if (Arrays.equals(presetIdBytes, cmpArr)) {
					if (matchingIndex > 0) {
						logger.warn("Found several ID string matches!");
					}
					matchingIndex = i;
					logger.info("Found firmware \"" + PRESET_ID_STRING + "\" string at: " + i);
				}
			}
			if (matchingIndex < 0) {
				logger.fatal("Could not find firmware \"" + PRESET_ID_STRING + "\" string");
			} else {
				/* Write mote-unique info to firmware */
				logger.info("Writing preset ID info to firmware");

				/* TODO uIPv4, uIPv6, MAC, ds2411 ... */

				/* Preset ID: flag */
				writeInt(0, firmware, matchingIndex+0);

				/* Preset ID: node_id */
				int nodeID = moteComPort.mote.getID();
				writeInt(nodeID, firmware, matchingIndex+2);
			}

			/* Write mote-unique firmware to disk */
			if (!ArrayUtils.writeToFile(firmwareFile, firmware)) {
				logger.fatal("Failed writing firmware " + firmwareFile.getName());
				return false;
			}

			/* Convert to ihex */
			File ihexFile = new File(firmwareFile.getName() + ".ihex");
			MessageList errors = new MessageList();
			try {
				CompileContiki.compile(
						"msp430-objcopy -I elf32-msp430 " + firmwareFile.getName() + " -O ihex " + ihexFile.getName(),
						null,
						ihexFile,
						new File("."),
						null,
						null,
						errors,
						true
						);
			} catch (Exception e) {
				logger.fatal("Failed conversion " + firmwareFile.getName() + " -> " + ihexFile.getName() + ": " + e.getMessage());
				MessageContainer[] msgs = errors.getMessages();
				for (MessageContainer msg: msgs) {
					logger.fatal(msg.message);
				}
				return false;
			}
			moteComPort.firmware = ihexFile;
			logger.info("Mote-unique ihex: " + moteComPort.mote + " -> " + moteComPort.firmware.getAbsolutePath());
		}
		return true;
	}

	private ArrayList<Process> processes = new ArrayList<Process>();
	private boolean abort = false;
	private void abortAllCommands() {
		abort = true;
		for (Process p: processes) {
			/* Try to kill processes - may not work on all platforms */
			p.destroy();
		}
		processes.clear();
	}

	private boolean executeProgrammingCommands(MoteCOMPort[] motes) {
		abort = false;
		/* Show progress bar */
		final JDialog progressDialog = new JDialog(
				(Window)GUI.getTopParentContainer(), 
				"Programming motes"
				) {
			private static final long serialVersionUID = -7483058587963650440L;
			public void dispose() {
				super.dispose();
				abortAllCommands();
			}
		};
		progressDialog.setDefaultCloseOperation(JDialog.DISPOSE_ON_CLOSE);
		final JProgressBar progressBar = new JProgressBar(0, 100);
		final MessageList output = new MessageList();
		progressBar.setValue(0);
		progressBar.setMaximum(motes.length);
		progressBar.setString("Programming motes...");
		progressBar.setStringPainted(true);
		progressBar.setIndeterminate(false);
		progressDialog.getContentPane().add(BorderLayout.NORTH, progressBar);
		progressDialog.getContentPane().add(BorderLayout.CENTER, new JScrollPane(output));
		progressDialog.setSize(550, 450);
		progressDialog.setLocationRelativeTo((Window)GUI.getTopParentContainer());
		new RunnableInEDT<Boolean>() {
			public Boolean work() {
				progressDialog.setVisible(true);
				return true;
			}
		}.invokeAndWait();

		ArrayList<Thread> threads = new ArrayList<Thread>();
		for (final MoteCOMPort moteComPort: motes) {
			if (moteComPort.firmware == null) {
				continue;
			}
			if (moteComPort.progPort == null) {
				continue;
			}
			threads.add(new Thread() {
				public void run() {
					try {
						while (!abort) {
							String cmd = getResetCommand(moteComPort);
							Process p = null;
							if (cmd != null) {
								p = CompileContiki.compile(
										cmd,
										null,
										null,
										moteComPort.firmware.getParentFile(),
										null,
										null,
										output,
										false
										);
								processes.add(p);
								p.waitFor();
								processes.remove(p);
								Thread.sleep(1500);
								p = CompileContiki.compile(
										cmd,
										null,
										null,
										moteComPort.firmware.getParentFile(),
										null,
										null,
										output,
										false
										);
								processes.add(p);
								p.waitFor();
								processes.remove(p);
								Thread.sleep(1500);
							}

							cmd = getEraseCommand(moteComPort);
							if (cmd != null) {
								p = CompileContiki.compile(
										cmd,
										null,
										null,
										moteComPort.firmware.getParentFile(),
										null,
										null,
										output,
										false
										);
								processes.add(p);
								p.waitFor();
								processes.remove(p);
								Thread.sleep(1500);
							}

							cmd = getProgramCommand(moteComPort);
							if (cmd != null) {
								p = CompileContiki.compile(
										cmd,
										null,
										null,
										moteComPort.firmware.getParentFile(),
										null,
										null,
										output,
										false
										);
								processes.add(p);
								p.waitFor();
								processes.remove(p);
							}

							if (p != null && p.exitValue() == 0) {
								/* Success! We are done programming! */
								progressBar.setValue(progressBar.getValue()+1);
								progressBar.setString(moteComPort.mote + " @ " + moteComPort.serialPort + "/" + moteComPort.progPort + " done!");
								break;
							} else {
								progressBar.setString("Retrying " + moteComPort.mote + " @ " + moteComPort.serialPort + "/" + moteComPort.progPort);
							}
							Thread.sleep(2500); /* Pause at failure */
						}
					} catch (Exception e) {
						logger.fatal("Error: " + e.getMessage());
						e.printStackTrace();
						progressBar.setIndeterminate(false);
						progressBar.setValue(100);
					}
				}
			});
		}
		for (Thread t: threads) {
			t.start();
		}
		for (Thread t: threads) {
			try {
				t.join();
			} catch (InterruptedException e) {
			}
		}

		/* Hide progress bar */
		new RunnableInEDT<Boolean>() {
			public Boolean work() {
				progressDialog.setVisible(false);
				progressDialog.dispose();
				return true;
			}
		}.invokeAndWait();
		return true;
	}

	public String getMoteSerialPort(Mote mote) {
		for (MoteCOMPort moteComPort: moteComPorts) {
			if (moteComPort.mote == mote) {
				return moteComPort.serialPort;
			}
		}
		return null;
	}
	public void setMoteSerialPort(Mote mote, String comPort) {
		if (comPort != null) {
			freeSerialPort(comPort);
		}

		for (MoteCOMPort motePort: moteComPorts) {
			if (motePort.mote == mote) {
				motePort.serialPort = comPort;
				model.fireTableDataChanged();
				return;
			}
		}
	}
	private void freeSerialPort(String comPort) {
		if (comPort == null) {
			return;
		}
		for (MoteCOMPort motePort: moteComPorts) {
			if (comPort.equals(motePort.serialPort)) {
				motePort.serialPort = null;
				model.fireTableDataChanged();
				return;
			}
		}
	}

	public String getMoteProgPort(Mote mote) {
		for (MoteCOMPort moteComPort: moteComPorts) {
			if (moteComPort.mote == mote) {
				return moteComPort.progPort;
			}
		}
		return null;
	}
	public void setMoteProgPort(Mote mote, String comPort) {
		if (comPort != null) {
			freeProgPort(comPort);
		}

		for (MoteCOMPort motePort: moteComPorts) {
			if (motePort.mote == mote) {
				motePort.progPort = comPort;
				model.fireTableDataChanged();
				return;
			}
		}
	}
	private void freeProgPort(String comPort) {
		if (comPort == null) {
			return;
		}
		for (MoteCOMPort motePort: moteComPorts) {
			if (comPort.equals(motePort.progPort)) {
				motePort.progPort = null;
				model.fireTableDataChanged();
				return;
			}
		}
	}


	public String[] getAllCOMPorts() {
		String[] allPorts = new String[allCOMPorts.length];
		for (int i=0; i < allPorts.length; i++) {
			allPorts[i] = allCOMPorts[i].getName();
		}
		return allPorts;
	}

	public String getQuickHelp() {
		return "<b>Mote programmer</b><p>" +
				"Program locally connected devices.<p>" +
				"The default commands used are:<br>" +
				"<tt>$ make $(FIRMWARENOEXTENSION).reset FIRMWARE=$(FIRMWARE) PROGPORT=$(PROGPORT) SERIALPORT=$(SERIALPORT)</tt><br>" + 
				"<tt>$ make $(FIRMWARENOEXTENSION).erase FIRMWARE=$(FIRMWARE) PROGPORT=$(PROGPORT) SERIALPORT=$(SERIALPORT)</tt><br>" + 
				"<tt>$ make $(FIRMWARENOEXTENSION).upload FIRMWARE=$(FIRMWARE) PROGPORT=$(PROGPORT) SERIALPORT=$(SERIALPORT)</tt><p>" + 
				"These commands can be configured in the cooja.config file.<p>";
	}

	private AbstractAction updateAction = new AbstractAction("Update port list") {
		public void actionPerformed(java.awt.event.ActionEvent e) {
			model.fireTableDataChanged(); /* delect table */

			scanAllCOMPorts();
			updateAction.putValue(Action.NAME, "Rescan COM ports (" + allCOMPorts.length + ")");
			updateAction.putValue(Action.SHORT_DESCRIPTION, joinStrings(getAllCOMPorts()));

			for (Mote mote: simulation.getMotes()) {
				String moteComPort = getMoteSerialPort(mote);
				if (moteComPort == null) {
					continue;
				}
				CommPortIdentifier identifier = getCOMPortIdentifier(moteComPort);
				if (identifier == null) {
					logger.warn(mote + "'s com port was removed: " + moteComPort);
					freeSerialPort(moteComPort);
				}
			}
			for (Mote mote: simulation.getMotes()) {
				String moteComPort = getMoteProgPort(mote);
				if (moteComPort == null) {
					continue;
				}
				CommPortIdentifier identifier = getCOMPortIdentifier(moteComPort);
				if (identifier == null) {
					logger.warn(mote + "'s com port was removed: " + moteComPort);
					freeProgPort(moteComPort);
				}
			}
			model.fireTableDataChanged();
		};
	};
	private AbstractAction programAction = new AbstractAction("Program all") {
		private static final long serialVersionUID = 3015229183644265014L;
		public void actionPerformed(java.awt.event.ActionEvent e) {
			programMotesNow();
		};
	};
	private AbstractAction attachAction = new AbstractAction("Serial port mode") {
		private static final long serialVersionUID = 1758086047213934048L;
		public void actionPerformed(java.awt.event.ActionEvent e) {
			String[] options = new String[] { "Continue", "Abort" };
			int value = JOptionPane.showOptionDialog(GUI.getTopParentContainer(),
					"This operation will remove all motes, and\n" +
							"replace them with " + GUI.getDescriptionOf(RealSerialPortMoteType.class) + " motes!\n\n" +
							"Are you sure you want to continue?",
							"Replace simulation motes?",
							JOptionPane.YES_NO_OPTION, JOptionPane.WARNING_MESSAGE,
							null, options, options[0]);
			if (value != JOptionPane.YES_OPTION) {
				return;
			}

			MoteCOMPort[] clone = moteComPorts.toArray(new MoteCOMPort[0]);

			/* Remove motes and mote types */
			for (Mote m: simulation.getMotes()) {
				simulation.removeMote(m);
			}
			for (MoteType t: simulation.getMoteTypes()) {
				simulation.removeMoteType(t);
			}

			/* Add the same number of real serial port motes */
			simulation.getGUI().doCreateMoteType(RealSerialPortMoteType.class, false);
			MoteType moteType = simulation.getMoteTypes()[0];

			for (MoteCOMPort port: clone) {
				Mote mote = moteType.generateMote(simulation);
				mote.getInterfaces().getMoteID().setMoteID(port.mote.getID());
				mote.getInterfaces().getPosition().setCoordinates(
						port.mote.getInterfaces().getPosition().getXCoordinate(),
						port.mote.getInterfaces().getPosition().getYCoordinate(),
						port.mote.getInterfaces().getPosition().getZCoordinate()
						);
				if (port.serialPort != null) {
					for (MoteInterface mi: mote.getInterfaces().getInterfaces()) {
						if (!(mi instanceof RealSerialPortMoteInterface)) {
							continue;
						}
						((RealSerialPortMoteInterface)mi).setComPort(port.serialPort);
						break;
					}
				}
				simulation.addMote(mote);
			}

			simulation.getGUI().removePlugin(MoteProgrammerPlugin.this, false);
		};
	};

	final AbstractTableModel model = new AbstractTableModel() {
		private static final long serialVersionUID = 4100092300349411187L;
		public String getColumnName(int column) {
			if (column < 0 || column >= columns.length) {
				logger.fatal("Unknown column: " + column);
				return "";
			}
			return columns[column];
		}

		public int getRowCount() {
			return moteComPorts.size();
		}

		public int getColumnCount() {
			return columns.length;
		}

		public Object getValueAt(int row, int column) {
			if (row < 0 || row >= moteComPorts.size()) {
				logger.fatal("Unknown row: " + row);
				return "";
			}
			if (column < 0 || column >= columns.length) {
				logger.fatal("Unknown column: " + column);
				return "";
			}

			MoteCOMPort moteComPort = moteComPorts.get(row);
			if (column == IDX_MOTE) {
				return moteComPort.mote;
			}
			if (column == IDX_PROGPORT) {
				if (moteComPort.progPort == null) {
					return "[no comport]";
				}
				return moteComPort.progPort;
			}
			if (column == IDX_SERIALPORT) {
				if (moteComPort.serialPort == null) {
					return "[no comport]";
				}
				return moteComPort.serialPort;
			}
			if (column == IDX_PROGRAM) {
				return new Boolean(false);
			}

			logger.debug("Column data not implemented: " + column);
			return "?";
		}
		public void setValueAt(Object value, int rowIndex, int columnIndex) {
			if (columnIndex == IDX_PROGPORT) {
				if (!(value instanceof String)) {
					return;
				}
				if (value != null && value.equals("[no comport]")) {
					setMoteProgPort(moteComPorts.get(rowIndex).mote, null);
				} else {
					setMoteProgPort(moteComPorts.get(rowIndex).mote, (String) value);
				}
				return;
			}
			if (columnIndex == IDX_SERIALPORT) {
				if (!(value instanceof String)) {
					return;
				}
				setMoteSerialPort(moteComPorts.get(rowIndex).mote, (String) value);
				return;
			}
			if (columnIndex == IDX_PROGRAM) {
				programFirmwaresNow(new MoteCOMPort[] { moteComPorts.get(rowIndex) });
				return;
			}
			super.setValueAt(value, rowIndex, columnIndex);
		}
		public boolean isCellEditable(int row, int column) {
			if (column == IDX_PROGPORT) {
				return true;
			}
			if (column == IDX_SERIALPORT) {
				return true;
			}
			if (column == IDX_PROGRAM) {
				return true;
			}
			return false;
		}
		public Class<?> getColumnClass(int c) {
			if (c == IDX_PROGPORT) {
				return JComboBox.class;
			}
			if (c == IDX_SERIALPORT) {
				return JComboBox.class;
			}
			return getValueAt(0, c).getClass();
		}
	};

	public Collection<Element> getConfigXML() {
		ArrayList<Element> config = new ArrayList<Element>();
		Element element;

		for (Mote mote: simulation.getMotes()) {
			String moteComPort = getMoteSerialPort(mote);
			if (moteComPort == null) {
				continue;
			}
			element = new Element("serialport");
			element.setAttribute("mote", mote.toString());
			element.setAttribute("serialport", moteComPort);
			config.add(element);
		}
		for (Mote mote: simulation.getMotes()) {
			String moteComPort = getMoteProgPort(mote);
			if (moteComPort == null) {
				continue;
			}
			element = new Element("progport");
			element.setAttribute("mote", mote.toString());
			element.setAttribute("progport", moteComPort);
			config.add(element);
		}
		return config;
	}

	public boolean setConfigXML(Collection<Element> configXML, boolean visAvailable) {
		for (Element element: configXML) {
			String name = element.getName();
			if (name.equals("serialport")) {
				String moteDesc = (String) element.getAttribute("mote").getValue();
				String comPort = (String) element.getAttribute("serialport").getValue();
				for (Mote mote: simulation.getMotes()) {
					if (mote.toString().equals(moteDesc)) {
						setMoteSerialPort(mote, comPort);
						break;
					}
				}
			}
			if (name.equals("progport")) {
				String moteDesc = (String) element.getAttribute("mote").getValue();
				String comPort = (String) element.getAttribute("progport").getValue();
				for (Mote mote: simulation.getMotes()) {
					if (mote.toString().equals(moteDesc)) {
						setMoteProgPort(mote, comPort);
						break;
					}
				}
			}
		}
		return true;
	}

	public void closePlugin() {
		simulation.getEventCentral().removeMoteCountListener(newMotesListener);
		for (Mote m: simulation.getMotes()) {
			newMotesListener.moteWasRemoved(m);
		}
		GUI.getTopParentContainer().repaint();
	}

	public static CommPortIdentifier getCOMPortIdentifier(String comPort) {
		for (CommPortIdentifier port: allCOMPorts) {
			if (port.getName().equalsIgnoreCase(comPort)) {
				return port;
			}
		}
		return null;
	}
	public static void scanAllCOMPorts() {
		if (!initied) {
			initSerialLibrary();
		}

		Enumeration<CommPortIdentifier> portList = CommPortIdentifier.getPortIdentifiers();
		ArrayList<CommPortIdentifier> ports = new ArrayList<CommPortIdentifier>();
		while (portList.hasMoreElements()) {
			CommPortIdentifier port = portList.nextElement();
			ports.add(port);
		}
		allCOMPorts = ports.toArray(new CommPortIdentifier[0]);
	}
	private static boolean initied = false;
	private static void initSerialLibrary() {
		/* Prepare RXTX library */
		try {
			/* Update library path with DLL:s */
			File RXTX_DLL_DIR = getJARDirectory();
			if (!RXTX_DLL_DIR.exists()) {
				throw new FileNotFoundException("RXTX DLL directory not found: " + RXTX_DLL_DIR.getAbsolutePath());
			}
			String currentPath = System.getProperty("java.library.path");
			String rxtxPath = RXTX_DLL_DIR.getAbsolutePath() + File.pathSeparator;
			if (!currentPath.contains(rxtxPath)) {
				String libPath = rxtxPath + System.getProperty("java.library.path");
				System.setProperty("java.library.path", libPath);
				/*logger.info("Updated library path: " + System.getProperty("java.library.path"));*/
			} else {
				/*logger.info("Keeping library path: " + System.getProperty("java.library.path"));*/
			}

			/* Force searching in java.library.path */
			Field fieldSysPath = ClassLoader.class.getDeclaredField("sys_paths");
			if (fieldSysPath != null) {
				fieldSysPath.setAccessible(true);
				fieldSysPath.set(System.class.getClassLoader(), null);
			}
		} catch (IllegalAccessException e) {
			throw (RuntimeException) 
			new RuntimeException("Error at plugin load: " + e.getMessage()).initCause(e);
		} catch (NoSuchFieldException e) {
			throw (RuntimeException) 
			new RuntimeException("Error at plugin load: " + e.getMessage()).initCause(e);
		} catch (FileNotFoundException e) {
			throw (RuntimeException) 
			new RuntimeException("Error at plugin load: " + e.getMessage()).initCause(e);
		}
		initied = true;
	}

	private static File getJARDirectory() {
		try {
			return new File(MoteProgrammerSkin.class.getProtectionDomain().getCodeSource().getLocation().toURI()).getParentFile();
		} catch (URISyntaxException e) {
			return null;
		}
	}

	private static String joinStrings(String[] strings) {
		StringBuilder sb = new StringBuilder();
		for (String s: strings) {
			sb.append(s);
			sb.append(" ");
		}
		return sb.toString().trim();
	}

	private static String replaceVariables(String conf, MoteCOMPort moteComPort) {
		if (conf != null && !conf.trim().isEmpty()) {
			if (moteComPort.serialPort != null) {
				conf = conf.replace("$(SERIALPORT)", moteComPort.serialPort);
			}
			if (moteComPort.progPort != null) {
				conf = conf.replace("$(PROGPORT)", moteComPort.progPort);
			}
			if (moteComPort.firmware != null) {
				conf = conf.replace("$(FIRMWARE)", moteComPort.firmware.getName());

				String firmwareNoExtension = moteComPort.firmware.getName();
				if (firmwareNoExtension.lastIndexOf(".") > 0) {
					firmwareNoExtension = firmwareNoExtension.substring(0, firmwareNoExtension.lastIndexOf("."));
					conf = conf.replace("$(FIRMWARENOEXTENSION)", firmwareNoExtension);
				}
			}
			return conf;
		}
		return null;
	}
	public static String getResetCommand(MoteCOMPort moteComPort) {
		GUI gui = moteComPort.mote.getSimulation().getGUI();
		String conf = gui.getProjectConfig().getStringValue(MoteProgrammerPlugin.class, "CMD_RESET", null);
		return replaceVariables(conf, moteComPort);
	}

	public static String getEraseCommand(MoteCOMPort moteComPort) {
		GUI gui = moteComPort.mote.getSimulation().getGUI();
		String conf = gui.getProjectConfig().getStringValue(MoteProgrammerPlugin.class, "CMD_ERASE", null);
		return replaceVariables(conf, moteComPort);
	}

	public static String getProgramCommand(MoteCOMPort moteComPort) {
		GUI gui = moteComPort.mote.getSimulation().getGUI();
		String conf = gui.getProjectConfig().getStringValue(MoteProgrammerPlugin.class, "CMD_PROGRAM", null);
		return replaceVariables(conf, moteComPort);
	}
}
