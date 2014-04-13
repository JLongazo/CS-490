/*****************************************************************************
  DISCLAIMER:
  This software was produced in part by the National Institute of Standards
  and Technology (NIST), an agency of the U.S. government, and by statute is
  not subject to copyright in the United States.  Recipients of this software
  assume all responsibility associated with its operation, modification,
  maintenance, and subsequent redistribution.
*****************************************************************************/

package org.nist.usarui.handlers;

import org.nist.usarui.*;
import org.nist.usarui.ui.IridiumUI;

import java.util.*;

/**
 * Suppresses sensor messages and displays an info bar with the contents instead.
 * Handles most sensors!
 *
 * @author Stephen Carlson (NIST)
 */
public class SensorStatusHandler extends AbstractStatusHandler {
	/**
	 * The maximum number of entries shown in long data sets before truncation.
	 */
	public static final int MAX_ENTRIES = 10;
	
	public double currentX = 0;
	public double gx = 0;
	public double tx = 0;
	public double currentY = 0;
	public double gy = 0;
	public double ty = 0;
	public double direction;
	public double gd;
	public boolean going;
	public boolean push;
	public double range;
	private boolean p1 = true;
	private boolean right = false;
	private long sTime;
	private long tTime;
	public boolean tCheck = false;
	public boolean helpPending = false;
	private int aCount = 0;
	/**
	 * Returns the 3-vector with colors.
	 *
	 * @param vec the string value
	 * @param convert whether radian to degree conversion will be applied
	 * @return the value reformatted for display
	 */
	private static String color3Vector(String vec, boolean convert) {
		Vec3 vector = Utils.read3Vector(vec); String deg = "";
		vector = vector.radToDeg(convert);
		if (convert)
			deg = DEG_SIGN;
		return String.format("<font color=\"#990000\">%.2f</font>%s <font color=\"#009900\">" +
			"%.2f</font>%s <font color=\"#000099\">%.2f</font>%s", vector.getX(), deg,
			vector.getY(), deg, vector.getZ(), deg);
	}
	/**
	 * Converts the floating point value to a sensible screen value.
	 *
	 * @param value the string value
	 * @param convert whether radian to degree conversion will be applied
	 * @return the value reformatted for display
	 */
	private static String floatString(String value, boolean convert) {
		String out = null, token, deg; int index = 0; float f;
		if (value != null)
			try {
				StringTokenizer str = new StringTokenizer(value, ",");
				StringBuilder output = new StringBuilder(3 * value.length() / 2);
				// Convert comma delimited to screen (if there is only one value, this works too)
				while (str.hasMoreTokens() && index < MAX_ENTRIES) {
					token = str.nextToken().trim();
					f = Float.parseFloat(token);
					// Apply conversion to degrees if necessary
					if (convert) {
						f = (float)Math.toDegrees(f);
						deg = DEG_SIGN;
					} else
						deg = "";
					output.append(String.format("%.2f%s", f, deg));
					if (str.hasMoreTokens())
						output.append(", ");
					index++;
				}
				if (str.hasMoreTokens())
					output.append("...");
				out = output.toString();
			} catch (NumberFormatException e) {
				// Trim down long data sets to size
				if (value.length() >= 40)
					out = value.substring(0, 40);
				else
					out = value;
				out = Utils.htmlSpecialChars(out);
			}
		return out;
	}
	/**
	 * Gets the GPS data representation from the packet.
	 *
	 * @param packet the packet to parse
	 * @return the GPS data reformatted for display
	 */
	private static String getGPSData(USARPacket packet) {
		int latDeg, latMin, longDeg, longMin;
		String value, lat, lon; StringTokenizer str;
		if (packet.getParam("Fix").equals("1"))
			value = "<font color=\"#009900\">Fix";
		else
			value = "<font color=\"#990000\">Loss";
		value += "</font> (" + Utils.htmlSpecialChars(packet.getParam("Satellites")) + ") ";
		// GPS data: Latitude 39,20 Longitude -78,30
		if (packet.getParam("Latitude") != null)
			try {
				// Parse latitude
				str = new StringTokenizer(packet.getParam("Latitude"), ",");
				latDeg = Integer.parseInt(str.nextToken().trim());
				latMin = Integer.parseInt(str.nextToken().trim());
				lat = str.nextToken().trim().toUpperCase();
				// Parse longitude
				str = new StringTokenizer(packet.getParam("Longitude"), ",");
				longDeg = Integer.parseInt(str.nextToken().trim());
				longMin = Integer.parseInt(str.nextToken().trim());
				lon = str.nextToken().trim().toUpperCase();
				// Output
				value += String.format("%d%s %d' <i>%s</i>, %d%s %d' <i>%s</i>",
					latDeg, DEG_SIGN, latMin, lat, longDeg, DEG_SIGN, longMin, lon);
			} catch (NoSuchElementException ignore) {
			} catch (NumberFormatException ignore) { }
		return Utils.asHTML(value);
	}
	/**
	 * Converts the odometer value to a sensible screen value.
	 *
	 * @param vec the string value
	 * @param convert whether radian to degree conversion will be applied
	 * @return the value reformatted for display
	 */
	private static String odoString(String vec, boolean convert) {
		Vec3 vector = Utils.read3Vector(vec); String deg = "";
		if (convert) {
			// Convert only Z (heading)for(int i = 0; i < MAX_BIDS; i++){
			vector = new Vec3(vector.getX(), vector.getY(),
				(float)Math.toDegrees(vector.getZ()));
			deg = DEG_SIGN;
		}
		return Utils.asHTML(String.format("<b>X</b> %.2f, <b>Y</b> %.2f, <b>T</b> %.2f%s",
			vector.getX(),vector.getY(), vector.getZ(), deg));
	}
	/**
	 * Converts the touch sensor value to a sensible screen value.
	 *
	 * @param touch the string value
	 * @return the value reformatted for display
	 */
	private static String touchString(String touch) {
		String out;
		touch = touch.trim().toLowerCase();
		if (touch.equals("1") || touch.equals("true"))
			out = "<font color=\"990000\">Touch</font>";
		else
			out = "<font color=\"009900\">No Touch</font>";
		return Utils.asHTML(out);
	}

	/**
	 * Creates a new instance.
	 *
	 * @param ui the application managing this handler
	 */
	public SensorStatusHandler(IridiumUI ui) {
		super(ui);
	}
	public String getPrefix() {
		return "Sen_";
	}
	public boolean statusReceived(USARPacket packet) {
		boolean keep = true, deg = ui.isInDegrees();
		if (packet.getType().equals("SEN")) {
			// Update time
			String tm = packet.getParam("Time"), value, test, type, name;
			if (tm != null)
				try {
					ui.updateTime(Float.parseFloat(tm));
				} catch (NumberFormatException ignore) { }
			// Update value, using typical names (this is for the simple sensors)
			type = packet.getParam("Type");
			if (type == null) type = "Sensor";
			name = packet.getParam("Name");
			if (name == null) name = type;
			// Default bulk data
			value = packet.getParam("");
			if (value != null) value = Utils.asHTML(floatString(value, false));
			// Accelerometer
			test = packet.getParam("ProperAcceleration");
			if (test != null) value = Utils.asHTML(floatString(test, false));
			test = packet.getParam("Acceleration");
			if (test != null) value = Utils.asHTML(floatString(test, false));
			// Bumper
			test = packet.getParam("Touch");
			if (test != null) value = touchString(test);
			// Encoder
			test = packet.getParam("Tick");
			if (test != null) value = Utils.htmlSpecialChars(test);
			// GPS
			test = packet.getParam("Fix");
			if (test != null) value = getGPSData(packet);
			// IR2Sensor, IRSensor, RangeSensor, RangeScanner, Sonar, and subclasses
			test = packet.getParam("Range");
			
			boolean avoiding = false;
			double dist = Math.sqrt(Math.pow(currentX - gx, 2)+Math.pow(currentY - gy,2));
			if (test != null) {
				String rf[] = test.split(",");
				//value = Utils.asHTML(floatString(test, false));
				double min = 8;
				int index = 0;
				for(int i = 0; i < rf.length; i++){
					double check = Double.parseDouble(rf[i]);
					if(check < min){
						min = check;
						index = i;
					}
				}
				range = min;
				//ui.setCheck(Double.toString(range));
				boolean close = false;
				if( dist < 1){
					close = true;
				}
				if(aCount > 8 && ui.saOn){
					ui.requestAid("Stuck");
					aCount = 0;
				}
				if(range < 1 && push && going && !close && !ui.manual){
					avoiding = true;
					if(right){
						ui.sendMessage("DRIVE {Left 2} {Right -2}");
					}else {
						ui.sendMessage("DRIVE {Left -2} {Right 2}");
					}
					aCount++;
					try {
						Thread.sleep(500);
					} catch (InterruptedException e) {
						// TODO Auto-generated catch block
						e.printStackTrace();
					}
				}
			}
			// Odometer
			test = packet.getParam("Pose");
			if (test != null) value = odoString(test, deg);
			// Tachometer
			test = packet.getParam("Pos");
			if (test != null)
				value = Utils.asHTML("<b>Rotation</b> (" + floatString(test, deg) +
					"), <b>Velocity</b> (" + floatString(packet.getParam("Vel"), deg) + ")");
			// GroundTruth, INS
			test = packet.getParam("Location");
			if (test != null){
				String check[] = test.split(",");
				String check2[] = packet.getParam("Orientation").split(",");
				currentX = Double.parseDouble(check[0]);
				currentY = Double.parseDouble(check[1]);
				gd = Math.atan2(currentY - gy, currentX - gx);	
				if(gd < 0){
					gd += (2*Math.PI);
				}
				right = false;
				double rSpeed = .3;
				double lSpeed = .3;
				direction = Double.parseDouble(check2[2]);
				if(gd > direction){
					if(gd - direction < Math.PI){
						right = true;
					} 
				} else {
					if(direction - gd > Math.PI){
						right = true;
					}
				}
				if(!tCheck && going){
					sTime = System.currentTimeMillis();
					tTime = ((Math.round(dist) * 20000) + 20000 + sTime);
					tCheck = true;
				}
				if((System.currentTimeMillis() > tTime) && going && ui.saOn){
					if(!helpPending){
						ui.requestAid("Timeout");
					}
					ui.setCheck("" + (tTime - sTime));
					tCheck = false;
					helpPending = true;
				}
				if(going && !avoiding && !ui.stopped && !ui.manual && !helpPending){
					aCount = 0;
					if(push){
						if(Math.abs(direction - gd) > (Math.PI/2)){
							if(right){
								rSpeed-=.4;
							}else{
								lSpeed-=.4;
							}
						}else if(Math.abs(direction - gd) > (Math.sqrt(2)/2)){
							if(right){
								rSpeed-=.3;
							}else{
								lSpeed-=.3;
							}
						}else if(Math.abs(direction - gd) > 0){
							if(right){
								rSpeed-=.1;
							}else{
								lSpeed-=.1;
							}
						}
					}else{
						if(!(direction + .1 > gd && direction - .1 < gd)){
							if(right){
								lSpeed = .2;
								rSpeed = -.2;
							}else{
								lSpeed = -.2;
								rSpeed = .2;
							}
						}else{
							
							if(ui.coordinate){
								ui.notifyCompanion();
								if(!ui.cReady){
									lSpeed = 0;
									rSpeed = 0;
								}else{
									ui.nearComp = true;
									ui.canBid();
								}
							}else{
								ui.nearComp = true;
								ui.canBid();
							}
						}
					}
					if(currentX + .3 > gx && currentX - .3 < gx && currentY + .3 > gy && currentY - .3 < gy){
						ui.sendMessage("DRIVE {Left 0.0} {Right 0.0}");
						if(push){
							ui.push(gx, ty, true);
							push = false;
							//ui.setCheck("pushing");
						}else{
							going = false;
							ui.complete();
							tCheck = false;
							helpPending = false;
						}
					}else{
						if(dist > 5 && push){
							lSpeed *= 1.5;
							rSpeed *= 1.5;
						}else if(dist < 1 && push){
							//lSpeed*=.7;
							//rSpeed*=.7;
						}
						ui.sendMessage("DRIVE {Left " + Double.toString(lSpeed) + "} {Right " + Double.toString(rSpeed) +"}");
					}
				}
				if(ui.manual){
					tCheck = false;
					ui.updateHUB(currentX, currentY);
				}
				value = Utils.asHTML("<b>At</b> (" + color3Vector(test, false) +
					"), <b>facing</b> (" + color3Vector(packet.getParam("Orientation"), deg) +
					")");
			}
			// If still empty, single-keyed sensors can be handled here
			if (value == null) {
				List<String> keys = new ArrayList<String>(packet.getParams().keySet());
				keys.remove("Time");
				keys.remove("Type");
				keys.remove("Name");
				if (keys.size() == 1)
					value = Utils.htmlSpecialChars(packet.getParam(keys.get(0)));
			}
			// Send whatever we got
			if (value != null)
				setInformation(type, name, value);
			keep = false; // was set to false by S. Carlson, set to true to see all messages.
		}
		return keep;
	}
	
	public void forceComplete(){
		going = false;
		ui.complete();
		tCheck = false;
		helpPending = false;
	}
}