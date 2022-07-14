package ink.kscope.device.imshelper

import android.annotation.SuppressLint
import android.content.Context
import android.net.ConnectivityManager
import android.net.Network
import android.net.NetworkCapabilities
import android.net.NetworkRequest
import android.os.Parcel
import android.os.ServiceManager
import android.util.Log
import java.lang.ref.WeakReference

@SuppressLint("StaticFieldLeak")
object Ims: EntryStartup {
    val networkListener = object: ConnectivityManager.NetworkCallback() {
        override fun onAvailable(network: Network) {
            Log.i("ImsHelper", "Network $network is available!")
        }
        override fun onCapabilitiesChanged(network: Network, networkCapabilities: NetworkCapabilities) {
            Log.i("ImsHelper", "Received info about network $network, got $networkCapabilities")
        }
    }

    val nwRequest = NetworkRequest.Builder()
            .addCapability(NetworkCapabilities.NET_CAPABILITY_IMS)
            .build()

    val mHidlService = android.hidl.manager.V1_0.IServiceManager.getService()
    val gotSPRD = mHidlService.get("vendor.sprd.hardware.radio@1.0::IExtRadio", "slot1")

    override fun startup(ctxt: Context) {
        val cm = ctxt?.getSystemService(ConnectivityManager::class.java)
        cm.requestNetwork(nwRequest, networkListener)
    }
}
