package com.awxkee.jxlcoder

import android.graphics.Bitmap
import android.graphics.Matrix
import android.graphics.drawable.Drawable
import android.os.Build
import android.os.Bundle
import androidx.activity.ComponentActivity
import androidx.activity.compose.setContent
import androidx.compose.foundation.Image
import androidx.compose.foundation.layout.fillMaxSize
import androidx.compose.foundation.layout.fillMaxWidth
import androidx.compose.foundation.lazy.LazyColumn
import androidx.compose.material3.MaterialTheme
import androidx.compose.material3.Surface
import androidx.compose.material3.Text
import androidx.compose.runtime.Composable
import androidx.compose.runtime.LaunchedEffect
import androidx.compose.runtime.mutableStateListOf
import androidx.compose.runtime.remember
import androidx.compose.ui.Modifier
import androidx.compose.ui.graphics.asImageBitmap
import androidx.compose.ui.layout.ContentScale
import androidx.compose.ui.tooling.preview.Preview
import androidx.lifecycle.lifecycleScope
import com.awxkee.jxlcoder.animation.AnimatedDrawable
import com.awxkee.jxlcoder.animation.JxlAnimatedStore
import com.awxkee.jxlcoder.ui.theme.JXLCoderTheme
import com.bumptech.glide.integration.compose.ExperimentalGlideComposeApi
import com.google.accompanist.drawablepainter.rememberDrawablePainter
import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.launch
import okio.buffer
import okio.source
import java.util.UUID

class MainActivity : ComponentActivity() {
    @OptIn(ExperimentalGlideComposeApi::class)
    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)

//        val buffer1 = this.assets.open("hdr_cosmos.jxl").source().buffer().readByteArray()
//        assert(JxlCoder.isJXL(buffer1))
//        assert(JxlCoder().getSize(buffer1) != null)
//        val iccCosmosImage = JxlCoder().decode(buffer1)
//        val buffer2 = this.assets.open("second_jxl.jxl").source().buffer().readByteArray()
//        assert(JxlCoder.isJXL(buffer2))
//        assert(JxlCoder().getSize(buffer2) != null)
//        val buffer3 = this.assets.open("alpha_jxl.jxl").source().buffer().readByteArray()
//        assert(JxlCoder.isJXL(buffer3))
//        assert(JxlCoder().getSize(buffer3) != null)


        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.O) {
//            val buffer4 = this.assets.open("happy_india.jxl").source().buffer().readByteArray()
//            assert(JxlCoder.isJXL(buffer4))
//            val largeImageSize = JxlCoder().getSize(buffer4)
//            assert(largeImageSize != null)
//
//            val image10Bit = image //.copy(Bitmap.Config.RGBA_F16, true)
////            image10Bit.setColorSpace(ColorSpace.get(ColorSpace.Named.DCI_P3))

//            val decompressedImage =
//                JxlCoder().decodeSampled(
//                    buffer4, largeImageSize!!.width * 2, largeImageSize.height * 2,
//                    preferredColorConfig = PreferredColorConfig.RGBA_F16
//                )
//
////            val first = JxlCoder().decode(
////                this.assets.open("happy_india.jxl").source().buffer()
////                    .readByteArray(),
////                preferredColorConfig = PreferredColorConfig.RGB_565
////            )
////
////            val resized = getResizedBitmap(first, decompressedImage.width, decompressedImage.height)
////            first.recycle()
//
//            val compressedBuffer = JxlCoder().encode(
//                decompressedImage,
//                colorSpace = JxlColorSpace.RGB,
//                compressionOption = JxlCompressionOption.LOSSY,
//                effort = 4,
//                quality = 90,
//            )
//
//            val image =
//                JxlCoder().decode(
//                    compressedBuffer,
//                    preferredColorConfig = PreferredColorConfig.RGB_565
//                )

//            val encoder = JxlAnimatedEncoder(
//                width = decompressedImage.width,
//                height = decompressedImage.height,
//            )
//            encoder.addFrame(decompressedImage, 2000)
//            encoder.addFrame(resized, 2000)
//            val compressedBuffer: ByteArray = encoder.encode()

//            val image = JxlCoder().decode(compressedBuffer, preferredColorConfig = PreferredColorConfig.RGBA_8888)

//            val animatedImage = JxlAnimatedImage(compressedBuffer)
//            val drawable = animatedImage.animatedDrawable

            setContent {
                JXLCoderTheme {
                    var imagesArray = remember {
                        mutableStateListOf<Bitmap>()
                    }
                    var drawables = remember {
                        mutableStateListOf<Drawable>()
                    }
                    LaunchedEffect(key1 = Unit, block = {
                        lifecycleScope.launch(Dispatchers.IO) {

//                            val bufferPng = assets.open("lin.png").source().buffer().readByteArray()
//                            val bitmap = BitmapFactory.decodeByteArray(bufferPng, 0, bufferPng.size)
//                            lifecycleScope.launch {
//                                drawables.add(BitmapDrawable(resources, bitmap))
//                            }
//                            val jxlBuffer = JxlCoder.encode(bitmap)
//                            val animated = JxlCoder.decode(jxlBuffer)
//                            lifecycleScope.launch {
//                                drawables.add(BitmapDrawable(resources, animated))
//                            }

//                            val buffer4 = assets.open("its_totally_safe.gif").source().buffer().readByteArray()
//                            val jxlBuffer = JxlCoder.Convenience.gif2JXL(buffer4, quality = 55)
//                            val animated = JxlAnimatedImage(jxlBuffer)
//                            val drawable = AnimatedDrawable(JxlAnimatedStore(animated))
//                            lifecycleScope.launch {
//                                drawables.add(drawable)
//                            }
////
//                            val buffer5 = assets.open("elephant.png").source().buffer().readByteArray()
//                            val jxlBufferPNG = JxlCoder.Convenience.apng2JXL(buffer5, quality = 55)
//                            val animated1 = JxlAnimatedImage(jxlBufferPNG)
//                            val drawable1 = AnimatedDrawable(JxlAnimatedStore(animated1))
//                            lifecycleScope.launch {
//                                drawables.add(drawable1)
//                            }
//                            var assets = (this@MainActivity.assets.list("") ?: return@launch).toList()
//                            for (asset in assets) {
//                                try {
//                                    val buffer4 =
//                                        this@MainActivity.assets.open(asset).source().buffer()
//                                            .readByteArray()
//
//                                    val largeImageSize = JxlCoder.getSize(buffer4)
//                                    if (largeImageSize != null) {
//                                        var srcImage = JxlCoder.decodeSampled(
//                                            buffer4,
//                                            largeImageSize.width / 3,
//                                            largeImageSize.height / 3,
//                                            preferredColorConfig = PreferredColorConfig.RGBA_8888,
//                                            com.awxkee.jxlcoder.ScaleMode.FIT,
//                                            JxlResizeFilter.BICUBIC,
//                                            toneMapper = JxlToneMapper.LOGARITHMIC,
//                                        )
//                                        lifecycleScope.launch {
//                                            imagesArray.add(srcImage)
//                                        }
//                                    }
//                                } catch (e: Exception) {
//                                    if (e !is FileNotFoundException) {
//                                        throw e
//                                    }
//                                }
//                            }
                        }
                    })
                    // A surface container using the 'background' color from the theme
                    Surface(
                        modifier = Modifier.fillMaxSize(),
                        color = MaterialTheme.colorScheme.background
                    ) {
//                        AsyncImage(
//                            model = "https://pdfconverter1984.blob.core.windows.net/simple/wide_gamut.jxl",
//                            contentDescription = null,
//                            imageLoader = ImageLoader.Builder(this)
//                                .components {
//                                    add(JxlDecoder.Factory())
//                                }
//                                .build()
//                        )

//                        Image(
//                            bitmap = image.asImageBitmap(),
////                            painter = rememberDrawablePainter(drawable = drawable),
//                            modifier = Modifier
//                                .fillMaxWidth()
//                                .fillMaxHeight(),
//                            contentScale = ContentScale.FillWidth,
//                            contentDescription = "ok"
//                        )
                        LazyColumn(
                            modifier = Modifier
                                .fillMaxWidth()
                        ) {
                            items(imagesArray.count(), key = {
                                return@items UUID.randomUUID().toString()
                            }) {
                                Image(
                                    bitmap = imagesArray[it].asImageBitmap(),
                                    modifier = Modifier.fillMaxWidth(),
                                    contentScale = ContentScale.FillWidth,
                                    contentDescription = "ok"
                                )
                            }

                            items(drawables.count(), key = {
                                return@items UUID.randomUUID().toString()
                            }) {
                                Image(
                                    painter = rememberDrawablePainter(drawable = drawables[it]),
                                    modifier = Modifier.fillMaxWidth(),
                                    contentScale = ContentScale.FillWidth,
                                    contentDescription = "ok"
                                )
                            }
                        }

//                        GlideImage(
//                            model = "https://wh.aimuse.online/preset/hdr_cosmos.jxl",
//                            contentDescription = ""
//                        )
                    }
                }
            }
        }
    }
}

fun getResizedBitmap(bm: Bitmap, newWidth: Int, newHeight: Int): Bitmap {
    val width = bm.width
    val height = bm.height
    val scaleWidth = newWidth.toFloat() / width
    val scaleHeight = newHeight.toFloat() / height
    val matrix = Matrix()
    matrix.postScale(scaleWidth, scaleHeight)
    return Bitmap.createBitmap(
        bm, 0, 0, width, height, matrix, false
    )
}

@Composable
fun Greeting(name: String, modifier: Modifier = Modifier) {
    Text(
        text = "Hello $name!",
        modifier = modifier
    )
}

@Preview(showBackground = true)
@Composable
fun GreetingPreview() {
    JXLCoderTheme {
        Greeting("Android")
    }
}