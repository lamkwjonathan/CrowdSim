﻿using System.Collections;
using System.Collections.Generic;
using UnityEngine;

public class ResetPos : MonoBehaviour
{
    // Start is called before the first frame update
    void Start()
    {
        
    }

    // Update is called once per frame
    void Update()
    {
        transform.localPosition = new Vector3(0f, 0f, 0f);
        transform.localRotation = new Quaternion(0f, 0f, 0f,0f);
    }
}
