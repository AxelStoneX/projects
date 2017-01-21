using UnityEngine;
using System.Collections;

public class Side_Walls_Script : MonoBehaviour 
{	
	void OnTriggerEnter2D (Collider2D hitInfo) 
	{
		if (hitInfo.name == "Ball")
		{
			string wallName = transform.name;
			GetComponent<AudioSource>().Play ();
			GameManager.Score(wallName);
			hitInfo.gameObject.SendMessage("ResetBall");
		}
	}
}
