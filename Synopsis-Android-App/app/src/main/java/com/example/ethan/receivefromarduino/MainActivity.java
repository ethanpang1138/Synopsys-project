package com.example.ethan.receivefromarduino;

import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.lang.reflect.Method;
import java.util.Arrays;
import java.util.UUID;

import com.example.bluetooth1.R;

import android.annotation.SuppressLint;
import android.app.Activity;
import android.bluetooth.BluetoothAdapter;
import android.bluetooth.BluetoothDevice;
import android.bluetooth.BluetoothSocket;
import android.content.Intent;
import android.os.Build;
import android.os.Bundle;
import android.os.Handler;
import android.os.SystemClock;
import android.util.Log;
import android.view.View;
import android.view.View.OnClickListener;
import android.view.animation.Animation;
import android.view.animation.AnimationUtils;
import android.view.animation.RotateAnimation;
import android.widget.Button;
import android.widget.ImageView;
import android.widget.TextView;
import android.widget.Toast;
 
public class MainActivity extends Activity {
  private static final String TAG = "bluetooth2";
   
  TextView txtDirectionTitle, txtArduino;
  Handler h;
  String prevMsg = "";
  float curDegree = 0f;

  final String endFlag = "\r\n";
  final int RECIEVE_MESSAGE = 1;		// Status  for Handler
  private BluetoothAdapter btAdapter = null;
  private BluetoothSocket btSocket = null;
  
  private ConnectedThread mConnectedThread;
   
  // SPP UUID service
  private static final UUID MY_UUID = UUID.fromString("00001101-0000-1000-8000-00805F9B34FB");
 
  // MAC-address of Bluetooth module (you must edit this line)
  private static String address = "98:D3:31:FC:80:EC";
  private static String address0 = "20:18:01:22:81:65";

  final String[] DIRECTIONS = {"right", "front-right", "front", "front-left", "left", "back-left", "back", "back-right"};
  final float[] ANGLES = {0, 45, 90, 135, 180, 225, 270, 315};

  /** Called when the activity is first created. */
  @SuppressLint("HandlerLeak")
  @Override
  public void onCreate(Bundle savedInstanceState) {
    super.onCreate(savedInstanceState);
 
    setContentView(R.layout.activity_main);

    txtArduino = findViewById(R.id.txtArduino);
    txtDirectionTitle = findViewById(R.id.txtDirectionTitle);

    h = new Handler() {
    	@SuppressLint("SetTextI18n")
        public void handleMessage(android.os.Message msg) {
    		switch (msg.what) {
            case RECIEVE_MESSAGE:													// if receive massage
                StringBuilder sb = (StringBuilder) msg.obj;
                int endOfLineIndex = sb.indexOf(endFlag);
                if (endOfLineIndex > 2) {
                    String id = sb.substring(0,2); // check the ID
                    String sbprint = sb.substring(2, sb.indexOf(endFlag)); //remove the ID

                    Log.i("ID ", id);
                    Log.i("data to display ",sbprint);

                    if(id.equals("00")){
                        txtArduino.setText(sbprint);
                        if(!prevMsg.equals(sbprint)){ // only rotate if there is a different direction
                            rotateArrow(sbprint);
                        }
                        prevMsg = sbprint;
                    }else if(id.equals("01")){
                        txtArduino.setText(sbprint);

                    }
                }
            	break;
    		}
        }
	};
     
    btAdapter = BluetoothAdapter.getDefaultAdapter();		// get Bluetooth adapter
    checkBTState();
  }

  private void rotateArrow(String msg){
    ImageView arrow = findViewById(R.id.arrow2);
    float degree = 0f;
    float offset = 90f;
    if(msg.equals("no sound")){
        RotateAnimation anim = new RotateAnimation(curDegree, 0, Animation.RELATIVE_TO_SELF, .5f, Animation.RELATIVE_TO_SELF, .5f);
        anim.setDuration(500);
        anim.setFillAfter(true);
        arrow.startAnimation(anim);
        curDegree = 0;
    }
    for(int i = 0; i < DIRECTIONS.length; i++){
        if(DIRECTIONS[i].equals(msg)){
            degree = ANGLES[i] - offset;
        }
    }
    RotateAnimation anim = new RotateAnimation(curDegree, -degree, Animation.RELATIVE_TO_SELF,0.5f, Animation.RELATIVE_TO_SELF, 0.5f);
    anim.setDuration(500);
    anim.setFillAfter(true);
    arrow.startAnimation(anim);
    curDegree = -degree;
  }
  
  private BluetoothSocket createBluetoothSocket(BluetoothDevice device) throws IOException {
      if(Build.VERSION.SDK_INT >= 10){
          try {
              final Method  m = device.getClass().getMethod("createInsecureRfcommSocketToServiceRecord", new Class[] { UUID.class });
              return (BluetoothSocket) m.invoke(device, MY_UUID);
          } catch (Exception e) {
              Log.e(TAG, "Could not create Insecure RFComm Connection",e);
          }
      }
      return  device.createRfcommSocketToServiceRecord(MY_UUID);
  }

  @Override
  public void onResume() {
    super.onResume();
 
    Log.d(TAG, "...onResume - try connect...");
   
    // Set up a pointer to the remote node using it's address.
    BluetoothDevice device = btAdapter.getRemoteDevice(address0);
   
    // Two things are needed to make a connection:
    //   A MAC address, which we got above.
    //   A Service ID or UUID.  In this case we are using the
    //     UUID for SPP.
    
	try {
		btSocket = createBluetoothSocket(device);
	} catch (IOException e) {
		errorExit("Fatal Error", "In onResume() and socket create failed: " + e.getMessage() + ".");
	}
    
    /*try {
      btSocket = device.createRfcommSocketToServiceRecord(MY_UUID);
    } catch (IOException e) {
      errorExit("Fatal Error", "In onResume() and socket create failed: " + e.getMessage() + ".");
    }*/
   
    // Discovery is resource intensive.  Make sure it isn't going on
    // when you attempt to connect and pass your message.
    btAdapter.cancelDiscovery();
   
    // Establish the connection.  This will block until it connects.
    Log.d(TAG, "...Connecting...");
    try {
      btSocket.connect();
      Log.d(TAG, "....Connection ok...");
    } catch (IOException e) {
      try {
        btSocket.close();
      } catch (IOException e2) {
        errorExit("Fatal Error", "In onResume() and unable to close socket during connection failure" + e2.getMessage() + ".");
      }
    }
     
    // Create a data stream so we can talk to server.
    Log.d(TAG, "...Create Socket...");

    mConnectedThread = new ConnectedThread(btSocket);
    mConnectedThread.start();
  }
 
  @Override
  public void onPause() {
    super.onPause();
 
    Log.d(TAG, "...In onPause()...");
  
    try     {
      btSocket.close();
    } catch (IOException e2) {
      errorExit("Fatal Error", "In onPause() and failed to close socket." + e2.getMessage() + ".");
    }
  }
   
  private void checkBTState() {
    // Check for Bluetooth support and then check to make sure it is turned on
    // Emulator doesn't support Bluetooth and will return null
    if(btAdapter==null) { 
      errorExit("Fatal Error", "Bluetooth not support");
    } else {
      if (btAdapter.isEnabled()) {
        Log.d(TAG, "...Bluetooth ON...");
      } else {
        //Prompt user to turn on Bluetooth
        Intent enableBtIntent = new Intent(BluetoothAdapter.ACTION_REQUEST_ENABLE);
        startActivityForResult(enableBtIntent, 1);
      }
    }
  }
 
  private void errorExit(String title, String message){
    Toast.makeText(getBaseContext(), title + " - " + message, Toast.LENGTH_LONG).show();
    finish();
  }
 
  private class ConnectedThread extends Thread {
	    private final InputStream mmInStream;
	    private final OutputStream mmOutStream;
	 
	    public ConnectedThread(BluetoothSocket socket) {
	        InputStream tmpIn = null;
	        OutputStream tmpOut = null;
	        // Get the input and output streams, using temp objects because
	        // member streams are final
	        try {
	            tmpIn = socket.getInputStream();
	            tmpOut = socket.getOutputStream();
	        } catch (IOException e) {
	            Log.d(TAG, "input stream failed");
            }
	        mmInStream = tmpIn;
	        mmOutStream = tmpOut;
	    }
	 
	    public void run() {
	        Log.d(TAG, "created connected thread");
	        // Keep listening to the InputStream until an exception occurs
	        while (true) try {
                byte[] buffer;
                int bytes = 0;
                int tmpBytes = 0;
                StringBuilder sb = new StringBuilder();
                String tmpMsg;
                do {
                    buffer = new byte[256];
                    tmpBytes = mmInStream.read(buffer);
                    bytes += tmpBytes;
                    tmpMsg = new String(buffer,0,tmpBytes);
                    Log.d("tmpMsg ", tmpMsg);
                    sb.append(tmpMsg);
                } while (sb.indexOf(endFlag) == -1);

                Log.d("array ", Arrays.toString(buffer));
                Log.d("bytes ", "" + bytes);
                Log.d("msg: ", sb.toString());

                h.obtainMessage(RECIEVE_MESSAGE, sb).sendToTarget(); // Send to message queue Handler
            } catch (IOException e) {
	            Log.d(TAG, e.getMessage());
                break;
            }
	    }
	 
	    /* Call this from the main activity to send data to the remote device */
	    public void write(String message) {
	    	Log.d(TAG, "...Data to send: " + message + "...");
	    	byte[] msgBuffer = message.getBytes();
	    	try {
	            mmOutStream.write(msgBuffer);
	        } catch (IOException e) {
	            Log.d(TAG, "...Error data send: " + e.getMessage() + "...");     
	    	}
	    }
	}
}