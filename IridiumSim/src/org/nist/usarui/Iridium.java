/*****************************************************************************
  DISCLAIMER:
  This software was produced in part by the National Institute of Standards
  and Technology (NIST), an agency of the U.S. government, and by statute is
  not subject to copyright in the United States.  Recipients of this software
  assume all responsibility associated with its operation, modification,
  maintenance, and subsequent redistribution.
*****************************************************************************/

package org.nist.usarui;

import java.io.*;
import java.net.*;
import java.util.*;

import org.nist.usarui.ui.IridiumUI;

/**
 * The Iridium connection manager and format handler
 *
 * @author Stephen Carlson (NIST)
 */
public class Iridium implements IridiumConnector {
	private final Properties config;
	private BufferedReader in;
	private BufferedReader in2;
	private final List<IridiumListener> listeners;
	private Writer out;
	private Writer out2;
	private Socket toUSAR;
	private Socket toHUB;
	private IridiumUI ui;

	/**
	 * Initializes the program.
	 */
	public Iridium() {
		config = new Properties();
		listeners = new ArrayList<IridiumListener>(5);
		loadConfig();
	}
	
	public void setUI(IridiumUI u){
		ui = u;
	}
	public void addIridiumListener(IridiumListener listener) {
		synchronized (listeners) {
			listeners.add(listener);
		}
	}
	public synchronized void connect(String hostPort) throws IOException {
		Socket temp; String host;
		disconnect();
		int port = 3000, index = hostPort.indexOf(':');
		if (index > 0) {
			host = hostPort.substring(0, index);
			try {
				port = Integer.parseInt(hostPort.substring(index + 1));
			} catch (NumberFormatException e) {
				throw new IOException("Host not found: " + hostPort);
			}
		} else
			host = hostPort;
		// Open socket
		temp = new Socket();
		temp.connect(new InetSocketAddress(host, port), 1000);
		temp.setSoTimeout(0);
		// Create socket input and output
		toUSAR = temp;
		in = new BufferedReader(new InputStreamReader(toUSAR.getInputStream()));
		out = new BufferedWriter(new OutputStreamWriter(toUSAR.getOutputStream()));
		// Start thread to handle socket messages
		Thread t = new Thread(new USARThread(), "USAR Messaging Thread");
		t.setPriority(Thread.MIN_PRIORITY);
		t.setDaemon(true);
		t.start();
		invokeEvent("connected");
	}
	public synchronized void disconnect() {
		if (isConnected()) {
			try {
				toUSAR.close();
				out.close();
				in.close();
			} catch (Exception ignore) { }
			toUSAR = null;
			out = null;
			in = null;
			invokeEvent("disconnect");
		}
	}
	
	//--------------------------------------------------------------------------------------------------------------------
	public synchronized void connect2(int port) throws IOException {
		Socket temp; String host; String hostPort= "localhost";
		disconnect2();
		int index = hostPort.indexOf(':');
		ServerSocket ss = new ServerSocket(port);
		if (index > 0) {
			host = hostPort.substring(0, index);
			try {
				port = Integer.parseInt(hostPort.substring(index + 1));
			} catch (NumberFormatException e) {
				throw new IOException("Host not found: " + hostPort);
			}
		} else
			host = hostPort;
		// Open socket
		Thread t2 = new Thread(new Timeout(), "Timeout Checker");
		t2.setPriority(Thread.MIN_PRIORITY);
		t2.setDaemon(true);
		//t2.start();
		ui.setCheck("waiting for connection");
		temp = ss.accept();
		// Create socket input and output
		toHUB = temp;
		in2 = new BufferedReader(new InputStreamReader(toHUB.getInputStream()));
		out2 = new BufferedWriter(new OutputStreamWriter(toHUB.getOutputStream()));
		ui.setCheck("connection established");
		//t2.stop();
		// Start thread to handle socket messages
		Thread t = new Thread(new USARThread2(), "USAR Messaging Thread 2");
		t.setPriority(Thread.MIN_PRIORITY);
		t.setDaemon(true);
		t.start();
		//invokeEvent("connected");
	}
	public synchronized void disconnect2() {
		if (isConnected2()) {
			ui.setCheck("disconnected");
			try {
				toHUB.close();
				out2.close();
				in2.close();
			} catch (Exception ignore) { }
			toHUB = null;
			out2 = null;
			in2 = null;
			//invokeEvent("disconnect");
		}
	}
	//-------------------------------------------------------------------------------------------------------------------
	public Properties getConfig() {
		return config;
	}
	public IridiumListener[] getIridiumListeners() {
		synchronized (listeners) {
			return listeners.toArray(new IridiumListener[listeners.size()]);
		}
	}
	/**
	 * Fires the specified event to all listeners.
	 *
	 * @param eventName the event that occurred
	 */
	public void invokeEvent(String eventName) {
		synchronized (listeners) {
			for (IridiumListener listener : listeners)
				listener.processEvent(eventName);
		}
	}
	/**
	 * Fires the specified packet to all listeners.
	 *
	 * @param packet the packet triggering the event
	 */
	public void invokePacket(USARPacket packet) {
		synchronized (listeners) {
			for (IridiumListener listener : listeners)
				listener.processPacket(packet);
		}
	}
	public synchronized boolean isConnected() {
		return toUSAR != null && toUSAR.isConnected() && !toUSAR.isClosed();
	}
	public synchronized boolean isConnected2() {
		return toHUB != null && toHUB.isConnected() && !toHUB.isClosed();
	}
	/**
	 * Try to load user config; if that fails, load the one from the JAR
	 */
	private void loadConfig() {
		File file = new File("iridium.properties");
		boolean found = false;
		if (file.canRead())
			try {
				// Attempt file first
				InputStream is = new FileInputStream(file);
				config.load(is);
				is.close();
				found = true;
			} catch (IOException ignore) { }
		if (!found)
			try {
				// If not file, load from JAR
				InputStream jis = getClass().getResourceAsStream("/iridium.properties");
				if (jis != null) {
					config.load(jis);
					jis.close();
					found = true;
				}
			} catch (IOException ignore) { }
		if (!found)
			// Hardcode defaults to alert user of this issue
			config.put("RawCommand0", "INIT {Could not read iridium.properties file}");
	}
	public void removeIridiumListener(IridiumListener listener) {
		synchronized (listeners) {
			listeners.remove(listener);
		}
	}
	public void sendMessage(String message) throws IOException {
		out.write(message);
		out.write("\r\n");
		out.flush();
	}

	/**
	 * Runs the thread which checks the socket for updates.
	 */
	private class USARThread implements Runnable {
		public void run() {
			String line;
			// Listener thread
			try {
				while (isConnected() && (line = in.readLine()) != null){
					if ((line = line.trim()).length() > 0)
						invokePacket(new USARPacket(line, true));
				}
			} catch (IOException ignore) {
			} finally {
				disconnect();
				
			}
		}
	}
	
	private class USARThread2 implements Runnable {
		public void run() {
			String line;
			// Listener thread
			try {
				while (isConnected2() && (line = in2.readLine()) != null){
					line = line.trim();
					processMessage(line);
				}
				ui.setCheck("nothing");
			} catch (IOException ignore) {
				ui.setCheck("whoops");
			} finally {
				ui.setCheck("whoops");
				disconnect2();
				
			}
		}

		private void processMessage(String line){
			String message[] = line.split("/");
			switch(message[0]){
			case "CONTROLLER":
				try {
					sendMessage("INIT {ClassName USARBotAPI.WorldController} {Location -1.2700, -0.9400, 1.4700} {Rotation 0.0000, 0.0000, 0.0000}");
					sendMessage("CONTROL {Type Create} {ClassName WCCrate} {Name crate1} {Location 1.0000, 1.0000, 0.0000} {Rotation 0.0000, 0.0000, 0.0000} {Scale 1.0000, 1.0000, 1.0000} {Physics RigidBody}");
					sendMessage("CONTROL {Type Create} {ClassName WCCrate} {Name crate2} {Location -4.0000, 3.0000, 0.0000} {Rotation 0.0000, 0.0000, 0.0000} {Scale 1.0000, 1.0000, 1.0000} {Physics RigidBody}");
					sendMessage("CONTROL {Type Create} {ClassName WCCrate} {Name crate3} {Location 2.0000, -6.0000, 0.0000} {Rotation 0.0000, 0.0000, 0.0000} {Scale 2.0000, 2.0000, 1.0000} {Physics RigidBody}");
					sendMessage("CONTROL {Type Create} {ClassName WCCrate} {Name crate4} {Location 7.0000, 6.0000, 0.0000} {Rotation 0.0000, 0.0000, 0.0000} {Scale 1.0000, 1.0000, 1.0000} {Physics RigidBody}");
					sendMessage("CONTROL {Type Create} {ClassName WCCrate} {Name crate5} {Location -5.0000, -2.0000, 0.0000} {Rotation 0.0000, 0.0000, 0.0000} {Scale 1.0000, 1.0000, 1.0000} {Physics RigidBody}");
				}catch (IOException e){
					ui.setCheck("error1 " + message[0]);
				}
				break;
			case "ROBOT":
				try {
					double x = Double.parseDouble(message[1]);
					double y = Double.parseDouble(message[2]);
					sendMessage("INIT {ClassName USARBot.BasicSkidRobot} {Location " + x + ", " + y + ", 1.4700} {Rotation 0.0000, 0.0000, 0.0000}");
					out2.write("A"+ x + "/" + y + "\n");
					out2.flush();
				}catch(IOException e){
					ui.setCheck("error2 " + message[0]);
				}
				break;
			case "PUSH":
				//tell robot to push a box have x and y coords, respectively
				//message at index 1 & 2 
				break;
			default:
				ui.setCheck("error3");
			}
		}
	}
	
	private class Timeout implements Runnable{
		@Override
		public void run() {
			long time = System.currentTimeMillis();
			long time2 = time;
			while (time2 - time < 10000){
				time2 = System.currentTimeMillis();
			}
			System.exit(0);
		}
		
	}
}