using UnityEngine;
using System.Collections;

public class Player_Script : MonoBehaviour {

	public KeyCode moveUp;
	public KeyCode moveDown;

	public float speed = 6;
	
	void Update () 
    {
		Vector2 tempVector;
		tempVector.x = 0;
		if(Input.GetKey(moveUp))
		{
			tempVector.y = speed;
		}
		else if(Input.GetKey (moveDown))
		{
			tempVector.y = -speed;
		}
		else
		{
			tempVector.y = 0;
		}
		GetComponent<Rigidbody2D>().velocity = tempVector;
	}
}
