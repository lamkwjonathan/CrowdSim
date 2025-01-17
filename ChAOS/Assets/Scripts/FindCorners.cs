using System.Collections;
using System.Collections.Generic;
using System.Collections.Specialized;
using UnityEngine;

public class FindCorners : MonoBehaviour
{

    List<Vector3> LocalVertices;
    List<Vector3> CornerVertices;
    List<int> CornerIDs = new List<int>() { 0, 10, 120, 110 };
    float radius = 1.0f;

    // Start is called before the first frame update
    void Start()
    {
        LocalVertices = new List<Vector3>(GetComponent<MeshFilter>().mesh.vertices);
        CornerVertices = new List<Vector3>();
        foreach(int id in CornerIDs)
        {
            CornerVertices.Add(transform.TransformPoint(LocalVertices[id]));
        }
        Debug.Log(this.ToString());
        int count = 1;
        foreach (Vector3 point in CornerVertices)
        {
            Debug.Log(this.ToString() + " " + count + " : " + point.ToString());
            count++;
        }
        
        LocalVertices.Clear();
        CornerVertices.Clear();
    }

    // Update is called once per frame
    void Update()
    {
        
    }

}
