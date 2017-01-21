using UnityEngine;
using System.Collections;

public class ArmRotation : MonoBehaviour {

	public int rotationOffset = 0;


	// Update is called once per frame
	void Update () {

		//Substracting the position of player fron the mouse position
		Vector3 difference = Camera.main.ScreenToWorldPoint (Input.mousePosition) - transform.position;  
		difference.Normalize (); //Normalizing the Vector - sum of the Vector will be equal to one

		float rotZ = Mathf.Atan2 (difference.y, difference.x) * Mathf.Rad2Deg; //Find the Angle in degrees
		transform.rotation = Quaternion.Euler (0f, 0f, rotZ + rotationOffset);
	
	}
}
