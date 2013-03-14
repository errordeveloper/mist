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
 *
 * $Id: QuickDeploySkin.java,v 1.3 2009-11-04 19:06:18 fros Exp $
 */

import java.awt.Color;
import java.awt.Graphics;
import java.awt.Point;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;

import javax.swing.JCheckBoxMenuItem;
import javax.swing.JMenuItem;
import javax.swing.JPopupMenu;

import org.apache.log4j.Logger;

import se.sics.cooja.ClassDescription;
import se.sics.cooja.GUI;
import se.sics.cooja.Mote;
import se.sics.cooja.Simulation;
import se.sics.cooja.interfaces.Position;
import se.sics.cooja.plugins.Visualizer;
import se.sics.cooja.plugins.VisualizerSkin;
import se.sics.cooja.plugins.Visualizer.MoteMenuAction;
import se.sics.cooja.plugins.Visualizer.SimulationMenuAction;

@ClassDescription("Quick deploy")
public class MoteProgrammerSkin implements VisualizerSkin {
  private static Logger logger = Logger.getLogger(MoteProgrammerSkin.class);

  private Simulation simulation = null;
  private Visualizer visualizer = null;

  public MoteProgrammerSkin() {
  }

  public void setActive(Simulation simulation, Visualizer vis) {
    this.simulation = simulation;
    this.visualizer = vis;

    visualizer.registerSimulationMenuAction(DeployAction.class);
    visualizer.registerMoteMenuAction(MapPortAction.class);
  }

  public void setInactive() {
    visualizer.unregisterSimulationMenuAction(DeployAction.class);
    visualizer.unregisterMoteMenuAction(MapPortAction.class);
  }

  public Color[] getColorOf(Mote mote) {
    return null;
  }

  public void paintBeforeMotes(Graphics g) {
  }

  public void paintAfterMotes(Graphics g) {
    g.setColor(Color.DARK_GRAY);
    MoteProgrammerPlugin plugin;
    try {
      plugin = getQuickDeployPlugin(simulation);
    } catch (RuntimeException e) {
      g.drawString("[quick deploy skin needs quick deploy plugin]", 0, 12);
      return;
    }

    /* Print comports (right side) */
    Mote[] allMotes = simulation.getMotes();
    for (Mote mote: allMotes) {
      Position pos = mote.getInterfaces().getPosition();
      Point pixel = visualizer.transformPositionToPixel(pos);

      String msg = plugin.getMoteSerialPort(mote);
      if (msg == null) {
        msg = "[no comport]";
      }

      g.drawString(msg, pixel.x + Visualizer.MOTE_RADIUS + 4, pixel.y + 4);
    }
  }

  /* VISUALIZER SKIN ACTIONS */
  public static class MapPortAction implements MoteMenuAction {
    private static Mote popupMote = null;
    private static ActionListener popupListener = new ActionListener() {
      public void actionPerformed(ActionEvent e) {
        JMenuItem menuItem = (JMenuItem) e.getSource();
        if (popupMote == null) {
          logger.warn("Unknown mote");
          return;
        }
        MoteProgrammerPlugin plugin = getQuickDeployPlugin(popupMote.getSimulation());
        plugin.setMoteSerialPort(popupMote, menuItem.getText());
        GUI.getTopParentContainer().repaint();
      }
    };
    public void doAction(Visualizer visualizer, Mote mote) {
      MoteProgrammerPlugin plugin = getQuickDeployPlugin(mote.getSimulation());
      String currentSelection = plugin.getMoteSerialPort(mote);
      JPopupMenu popup = new JPopupMenu("Select comport");
      JCheckBoxMenuItem menuItem = new JCheckBoxMenuItem("[no comport]");
      menuItem.addActionListener(popupListener);
      popup.add(menuItem);
      for (String port: plugin.getAllCOMPorts()) {
        menuItem = new JCheckBoxMenuItem(port);
        menuItem.addActionListener(popupListener);
        if (currentSelection != null && currentSelection.equals(port)) {
          menuItem.setSelected(true);
        }
        popup.add(menuItem);
      }

      Point p = visualizer.transformPositionToPixel(mote.getInterfaces().getPosition());
      popupMote = mote;
      popup.show(visualizer, p.x, p.y);
    }
    public String getDescription(Visualizer visualizer, Mote mote) {
      return "Quick deploy: Select comport";
    }
    public boolean isEnabled(Visualizer visualizer, Mote mote) {
      try {
        getQuickDeployPlugin(mote.getSimulation());
      } catch (RuntimeException e) {
        return false;
      }
      if (mote.getType().getContikiFirmwareFile() == null) {
        return false;
      }
      return true;
    }
  };
  public static class DeployAction implements SimulationMenuAction {
    public boolean isEnabled(Visualizer visualizer, Simulation simulation) {
      try {
        getQuickDeployPlugin(simulation);
      } catch (RuntimeException e) {
        return false;
      }
      return true;
    }
    public String getDescription(Visualizer visualizer, Simulation simulation) {
      return "Quick deploy: Upload code";
    }
    public void doAction(Visualizer visualizer, Simulation simulation) {
      MoteProgrammerPlugin plugin = getQuickDeployPlugin(simulation);
      plugin.programMotesNow();
    }
  };

  public Visualizer getVisualizer() {
    return visualizer;
  }

  private static MoteProgrammerPlugin getQuickDeployPlugin(Simulation simulation) {
    MoteProgrammerPlugin plugin = (MoteProgrammerPlugin)
    simulation.getGUI().getStartedPlugin(MoteProgrammerPlugin.class.getName());
    if (plugin == null) {
      throw new RuntimeException("No Quick deploy plugin found!");
    }
    return plugin;
  }
}