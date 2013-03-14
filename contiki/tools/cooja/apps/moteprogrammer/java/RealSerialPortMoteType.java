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
 *
 */

import java.awt.Container;

import org.apache.log4j.Logger;

import se.sics.cooja.AbstractionLevelDescription;
import se.sics.cooja.ClassDescription;
import se.sics.cooja.Mote;
import se.sics.cooja.MoteInterface;
import se.sics.cooja.MoteType;
import se.sics.cooja.RadioPacket;
import se.sics.cooja.Simulation;
import se.sics.cooja.interfaces.ApplicationSerialPort;
import se.sics.cooja.motes.AbstractApplicationMote;
import se.sics.cooja.motes.AbstractApplicationMoteType;

@ClassDescription("Real Serial Port Mote Type")
@AbstractionLevelDescription("Application level")
public class RealSerialPortMoteType extends AbstractApplicationMoteType {
  private static Logger logger = Logger.getLogger(RealSerialPortMoteType.class);

  public RealSerialPortMoteType() {
    super();
  }

  public RealSerialPortMoteType(String identifier) {
    super(identifier);
    setDescription("Real Serial Port Mote Type #" + identifier);
  }

  public boolean configureAndInit(Container parentContainer,
      Simulation simulation, boolean visAvailable) 
  throws MoteTypeCreationException {
    if (!super.configureAndInit(parentContainer, simulation, visAvailable)) {
      return false;
    }
    setDescription("Real Serial Port Mote Type #" + getIdentifier());
    return true;
  }

  public Mote generateMote(Simulation simulation) {
    return new RealSerialPortMote(this, simulation);
  }

  public Class<? extends MoteInterface>[] getMoteInterfaceClasses() {
    /* Replace default log interface */
    Class<? extends MoteInterface>[] classes = super.getMoteInterfaceClasses();
    for (int i=0; i < classes.length; i++) {
      if (classes[i].getName().equals(ApplicationSerialPort.class.getName())) {
        classes[i] = RealSerialPortMoteInterface.class;
        break;
      }
    }
    return classes;
  }

  public static class RealSerialPortMote extends AbstractApplicationMote {
    public RealSerialPortMote() {
      super();
    }
    public RealSerialPortMote(MoteType moteType, Simulation simulation) {
      super(moteType, simulation);
    }
    public void execute(long time) {
      /* Ignore */
    }
    public void receivedPacket(RadioPacket p) {
      /* Ignore */
    }
    public void sentPacket(RadioPacket p) {
      /* Ignore */
    }
    public String toString() {
      return "Real Serial Port " + getID();
    }
  }

}
