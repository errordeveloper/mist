import java.util.ArrayList;

import org.apache.log4j.Logger;
import org.bouncycastle.crypto.InvalidCipherTextException;
import org.bouncycastle.crypto.engines.AESEngine;
import org.bouncycastle.crypto.modes.CCMBlockCipher;
import org.bouncycastle.crypto.params.CCMParameters;
import org.bouncycastle.crypto.params.KeyParameter;

import se.sics.cooja.plugins.analyzers.PacketAnalyzer;
import se.sics.cooja.plugins.analyzers.RadioLoggerAnalyzerSuite;
import se.sics.cooja.util.StringUtils;

public class RadioLoggerAES implements RadioLoggerAnalyzerSuite {
  private static Logger logger = Logger.getLogger(NetstackAesAnalyzer.class);

  public String getDescription() {
    return "AES-128 netstack-aes.c";
  }

  public ArrayList<PacketAnalyzer> getAnalyzers() {
    ArrayList<PacketAnalyzer> arr = new ArrayList<PacketAnalyzer>();
    arr.add(new NetstackAesAnalyzer("thingsquare mist".getBytes()));
    return arr;
  }

  private class NetstackAesAnalyzer extends PacketAnalyzer {
    public static final int HEADER_LENGTH = 16; /* bytes */ 
    public static final int NONCE_LENGTH = 13; /* bytes */ 
    public static final int MIC_LENGTH = 4; /* bytes */

    /* TODO Add support for a master key */
    private byte[] key;

    public NetstackAesAnalyzer(byte[] key) {
      this.key = key;
    }

    public boolean matchPacket(Packet packet) {
      if (packet.size() < 1 /*LEN*/ + HEADER_LENGTH + MIC_LENGTH + 2 /*CRC*/) {
        return false;
      }

      /* TODO Do the actual decryption here */
      return true;
    }

    public int analyzePacket(Packet packet, StringBuffer brief, StringBuffer verbose) {
      int i;

      verbose.append("<b>Raw packet:</b><br><pre>");
      verbose.append(StringUtils.hexDump(packet.getPayload()));
      verbose.append("</pre><br>");

      packet.consumeBytesStart(1); /* Length byte */
      packet.consumeBytesEnd(2); /* CRC */

      byte[] header = new byte[HEADER_LENGTH];
      packet.copy(0, header, 0, HEADER_LENGTH);
      verbose.append("<b>Header:</b><br><pre>");
      verbose.append(StringUtils.hexDump(header));
      verbose.append("</pre><br>");

      /* Nonce creation: code ported from netstack-aes.c:177-183 */
      byte[] nonce = new byte[NONCE_LENGTH];
      i = 0;
      while(i < nonce.length) {
        int c = (nonce.length - i < HEADER_LENGTH) ? nonce.length - i : HEADER_LENGTH;
        System.arraycopy(header, 0, nonce, i, c);
        i += c;
      }
      verbose.append("<b>Nonce:</b><br><pre>");
      verbose.append(StringUtils.hexDump(nonce));
      verbose.append("</pre><br>");

      verbose.append("<b>Key:</b><br><pre>");
      verbose.append(StringUtils.hexDump(key));
      verbose.append("</pre><br>");

      CCMBlockCipher ccm = new CCMBlockCipher(new AESEngine());
      ccm.init(false, new CCMParameters(new KeyParameter(key), 8*MIC_LENGTH /* bits */, nonce, header));
      byte[] plaintext = new byte[ccm.getOutputSize(packet.getPayload().length-HEADER_LENGTH)];
      int len = ccm.processBytes(packet.getPayload(), HEADER_LENGTH, packet.getPayload().length-HEADER_LENGTH, plaintext, 0);
      try {
        ccm.doFinal(plaintext, len);
      } catch (IllegalStateException | InvalidCipherTextException e) {
        logger.warn("Error analyzing packet: " + e.getMessage(), e);
        return ANALYSIS_FAILED;
      }

      StringBuilder sb = new StringBuilder();
      sb.append("<html><b>Plaintext:</b><br><pre>");
      sb.append(StringUtils.hexDump(plaintext));
      sb.append("</pre><br>");
      verbose.insert(0, sb.toString());

      brief.append("AES");

      return ANALYSIS_OK_FINAL;
    }
  }

}
