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
import java.nio.channels.IllegalBlockingModeException;
import java.util.*;

import org.nist.usarui.handlers.SensorStatusHandler;
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
	private MulticastSocket toHUB;
	private IridiumUI ui;
	private int id;
	public double tx;
	public double ty;
	private int taskNum;
	private boolean working = false;
	private boolean controller = false;
	

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
		ui.setCheck("trying port");
		toHUB = new MulticastSocket(port);
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
		//Thread t2 = new Thread(new Timeout(), "Timeout Checker");
		//t2.setPriority(Thread.MIN_PRIORITY);
		//t2.setDaemon(true);
		//t2.start();
		ui.setCheck("waiting for connection");
		toHUB.connect(new InetSocketAddress("localhost",port));
		// Create socket input and output
		//toHUB = ss;
		//in2 = new BufferedReader(new InputStreamReader(toHUB.getInputStream()));
		//out2 = new BufferedWriter(new OutputStreamWriter(toHUB.getOutputStream()));
		ui.setCheck("connection established " + Integer.toString(port));
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
			} catch (Exception ignore) { }
			toHUB = null;
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
	
	public void sendHubMessage(String message){
		byte[] content = message.getBytes();
		DatagramPacket p = new DatagramPacket(content, content.length);
		try {
			toHUB.send(p);
		} catch (IOException e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
		}
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
			// Listener thread
			try {
				while(isConnected2()){
					byte[] m = new byte[15];
					DatagramPacket p = new DatagramPacket(m, m.length);
					//ui.setCheck("waiting");
					toHUB.receive(p);
					//ui.setCheck("recieved");
					processMessage(new String(p.getData(), "UTF-8"));
					//ui.setCheck("nothing");
				}
			} catch (IOException ignore) {
				ui.setCheck("");
			}
		}

		private void processMessage(String line){
			String message[] = line.split("/");
			//ui.setCheck(line);
			switch(message[0]){
			case "CONTROLLER":
				try {
					if(id == 4){
						ui.setCheck(line);
						controller = true;
						sendMessage("INIT {ClassName USARBotAPI.WorldController} {Location -1.2700, -0.9400, 1.4700} {Rotation 0.0000, 0.0000, 0.0000}");
						sendMessage("CONTROL {Type Create} {ClassName WCCrate} {Name crate1} {Location 1.0000, 4.0000, 0.0000} {Rotation 0.0000, 0.0000, 0.0000} {Scale 1.0000, 1.0000, 1.0000} {Physics RigidBody}");
						sendMessage("CONTROL {Type Create} {ClassName WCCrate} {Name crate2} {Location -4.0000, 5.0000, 0.0000} {Rotation 0.0000, 0.0000, 0.0000} {Scale 1.0000, 1.0000, 1.0000} {Physics RigidBody}");
						sendMessage("CONTROL {Type Create} {ClassName WCCrate} {Name crate3} {Location 2.0000, -6.0000, 0.0000} {Rotation 0.0000, 0.0000, 0.0000} {Scale 1.0000, 1.0000, 1.0000} {Physics RigidBody}");
						sendMessage("CONTROL {Type Create} {ClassName WCCrate} {Name crate4} {Location 7.0000, 6.0000, 0.0000} {Rotation 0.0000, 0.0000, 0.0000} {Scale 1.0000, 1.0000, 1.0000} {Physics RigidBody}");
						sendMessage("CONTROL {Type Create} {ClassName WCCrate} {Name crate5} {Location -5.0000, -2.0000, 0.0000} {Rotation 0.0000, 0.0000, 0.0000} {Scale 1.0000, 1.0000, 1.0000} {Physics RigidBody}");
					}
				}catch (IOException e){
					ui.setCheck("error1 " + message[0]);
				}
				break;
			case "ROBOT":
				try {
					
					if(id == Integer.parseInt(message[1]) && isConnected()){
						ui.setCheck(message[1]);
						double x = Double.parseDouble(message[2]);
						double y = Double.parseDouble(message[3]);
						sendMessage("INIT {ClassName USARBot.BasicSkidRobot} {Location " + x + ", " + y + ", 1.4700} {Rotation 0.0000, 0.0000, 0.0000}");
					}
				}catch(IOException e){
					ui.setCheck("error2 " + message[0]);
				}
				break;
			case "TASK":
				//ui.setCheck(line);
				if(!working && !controller){
					tx = Double.parseDouble(message[2]);
					ty = Double.parseDouble(message[3]);
					taskNum = Integer.parseInt(message[1]);
					double bid = ui.getBid(tx, ty);
					sendHubMessage("B/"+ Integer.toString(id) + "/" + Double.toString(bid) + "/");
				}
				break;
			case "WINNER":
				//ui.setCheck(line);
				if(id == Integer.parseInt(message[1]) && isConnected()){
					ui.push(tx, ty, false);
					working = true;
					sendHubMessage("G/");
					
				} else {
					tx = 0;
					ty = 0;
				}
				break;
			case "ESTOP":
				if(id == Integer.parseInt(message[1]) && isConnected()){
					if(ui.stopped){
						ui.stopped = false;
					}else{
						ui.stopped = true;
						ui.sendMessage("DRIVE {Left 0.0} {Right 0.0}");
					}
				}
            case "DRIVE":
                //Assuming skid
                ui.setCheck(message[1]);
                double right = Double.parseDouble(message[2]);
                double left = Double.parseDouble(message[3]);
                
                try{
                    sendMessage(String.format("DRIVE {Left %.2f} {Right %.2f}", left, right));
                } catch (IOException e){
                    ui.setCheck("Error2 " + message[0]);
                }
                break;
			default:
				//ui.setCheck("error3");
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
	
	public void setId(int i){
		id = i;
	}
	
	public String getId(){
		return Integer.toString(id);
	}
	
	public String getTN(){
		return Integer.toString(taskNum);
	}
	
	public void notWorking(){
		working = false;
	}
}