using System;
using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using UnityEngine.VFX;
using UnityEngine.UI;
using System.Runtime.InteropServices;

public class PointCloudLoader : MonoBehaviour
{
    const int TextureDimensionX = 1024;
    const int TextureDimensionY = 1024;

    public RawImage debugImage;

    [SerializeField]
    private int PointCount;

    public VisualEffect effect;
    public string PositionTextureName = "PositionMap";
    public string ColorTextureName = "ColorMap";

    Texture2D positionMap;
    Texture2D colorMap;

    int messasgeSize;
    byte[] message;


    short[] intArray = new short[TextureDimensionX * TextureDimensionY * 3];
    float[] positionArray = new float[TextureDimensionX * TextureDimensionY * 4];

    byte[] positionBytes = new byte[TextureDimensionX * TextureDimensionY * 4 * 4];
    byte[] colorBytes = new byte[TextureDimensionX * TextureDimensionY * 3]; 

    volatile bool textureNeedsUpdate = false;
    int currentPointCount = 0;

    public void LoadPoints(byte[] data, int length)
    {
        int header = 13;

        ulong timestamp = BitConverter.ToUInt64(data, 0);
        int pointCount = (int)BitConverter.ToUInt32(data, 8);
        bool hasColor = BitConverter.ToBoolean(data, 12);

        //Debug.Log("Loading points:: " + pointCount);
        PointCount = pointCount;
        Buffer.BlockCopy(data, header, intArray, 0, pointCount * 6);

        int srcIndex = 0;
        int destIndex = 0;

        //convert from int16 to float
        for (int i = 0; i < pointCount; i++)
        {         
            positionArray[destIndex] = (intArray[srcIndex] * 0.001f);
            positionArray[destIndex + 1] = (intArray[srcIndex + 1] * 0.001f);
            positionArray[destIndex + 2] = (intArray[srcIndex + 2] * 0.001f);
            positionArray[destIndex + 3] = 1;

            srcIndex += 3;
            destIndex += 4;
        }

        //fill texture using float positions array
        try
        {
            unsafe
            {
                //rgba positionArray
                int positionChunk = (pointCount * 4) * 4;
                int index = 0;
                fixed (float* p = &positionArray[0])
                {
                    //fill the whole texture as vfx graphic is randomly sampling
                    while (index < (positionBytes.Length - positionChunk))
                    {
                        Marshal.Copy(new IntPtr(p), positionBytes, index, positionChunk);
                        index += positionChunk;
                    }

                    //fill the remaining texture
                    Marshal.Copy(new IntPtr(p), positionBytes, index, (positionBytes.Length - index));
                }
            }
        }
        catch (Exception e)
        {
            Debug.LogError("Exception creating point texture:" + e.Message);
        }

        //fill color texture directly from payload
        if(hasColor)
        {
            try
            {
                unsafe
                {
                    //rbg raw data
                    int colorChunck = pointCount * 3;

                    int index = 0;
                    fixed (byte* b = &data[header + ((pointCount * 3) * 2)])
                    {
                        while (index < (colorBytes.Length - colorChunck))
                        {
                            Marshal.Copy(new IntPtr(b), colorBytes, index, colorChunck);
                            index += colorChunck;
                        }
                    }
                }
            }
            catch (Exception e)
            {
                Debug.LogError("Exception creating color texture:" + e.Message);
            }
        }

        currentPointCount = pointCount;
        textureNeedsUpdate = true;
    }


    void Start()
    {
        positionMap = new Texture2D(TextureDimensionX, TextureDimensionY, TextureFormat.RGBAFloat, false);
        positionMap.filterMode = FilterMode.Point;


        colorMap = new Texture2D(TextureDimensionX, TextureDimensionY, TextureFormat.RGB24, false);
        colorMap.filterMode = FilterMode.Point;

        if (debugImage)
        {
            debugImage.texture = positionMap;
        }

        if (effect)
        {
            effect.SetTexture("PositionMap", positionMap);
            effect.SetTexture("ColorMap", colorMap);
        }
    }

    public void UpdateEffect(VisualEffectAsset newEffect)
    {
        Debug.Log("Update Effect");

        effect.visualEffectAsset = newEffect;
        effect.SetTexture("PositionMap", positionMap);
        effect.SetTexture("ColorMap", colorMap);
        effect.Reinit();
    }


    void Update()
    {
        if (textureNeedsUpdate)
        {
            positionMap.LoadRawTextureData(positionBytes);
            positionMap.Apply();

            colorMap.LoadRawTextureData(colorBytes);
            colorMap.Apply();

            textureNeedsUpdate = false;
        }
    }
}
