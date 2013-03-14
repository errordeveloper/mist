/*
 * Copyright (c) 2011, Swedish Institute of Computer Science.
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

package se.sics.cooja.mspmote;

import java.io.File;

import org.apache.log4j.Logger;

import se.sics.cooja.MoteInterface;
import se.sics.cooja.Simulation;
import se.sics.cooja.mspmote.interfaces.Msp802154Radio;
import se.sics.mspsim.platform.GenericNode;
import se.sics.mspsim.platform.ti.Exp1101Node;
import se.sics.mspsim.platform.ti.Exp1120Node;
import se.sics.mspsim.platform.ti.Exp5438Node;

import com.thingsquare.cooja.mspsim.CC1101Radio;
import com.thingsquare.cooja.mspsim.CC1120Radio;

/**
 * @author Fredrik Osterlind
 */
public class Exp5438Mote extends MspMote {
  private static Logger logger = Logger.getLogger(Exp5438Mote.class);

  public GenericNode exp5438Node = null;
  private String desc = "";
  
  public Exp5438Mote(MspMoteType moteType, Simulation sim) {
    super(moteType, sim);
  }

  protected boolean initEmulator(File fileELF) {
	  /* Check radio type */
	  exp5438Node = null;
	  for (Class<? extends MoteInterface> clazz : getType().getMoteInterfaceClasses()) {
		  if (clazz == CC1101Radio.class) {
			  exp5438Node = new Exp1101Node();
			  desc = "Exp5438+CC1101";
			  break;
		  } else if (clazz == CC1120Radio.class) {
			  exp5438Node = new Exp1120Node();
			  desc = "Exp5438+CC1120";
			  break;
		  } else if (clazz == Msp802154Radio.class) {
			  exp5438Node = new Exp5438Node();
			  desc = "Exp5438+CC2420";
			  break;
		  }
	  }
	  if (exp5438Node == null) {
		  throw new IllegalStateException("unknown radio");
	  }
	  
    try {
      registry = exp5438Node.getRegistry();
      prepareMote(fileELF, exp5438Node);
    } catch (Exception e) {
      logger.fatal("Error when creating Exp5438 mote: ", e);
      return false;
    }
    return true;
  }

  public void idUpdated(int newID) {
  }

  public String toString() {
    return desc + " " + getID();
  }

}
