using UnityEngine;
using System.Collections;

public class Overlaping : MonoBehaviour {

	private GameObject player1;
	private Component sRenderer;

	void Start(){

		player1 = GameObject.FindGameObjectWithTag("Player");
		if (player1 == null)
			Debug.LogError ("Object with player tag had not been found!");
		sRenderer = player1.GetComponent<SpriteRenderer>();
		if (sRenderer == null)
			Debug.LogError ("Player has not sprite renderer component");
	}

	void OnTriggerStay2D(Collider2D other){

		if (other.name == "Player")
		{
			sRenderer.GetComponent<Renderer>().sortingOrder = 0;
		}

	}

	void OnTriggerExit2D(Collider2D other){

		if (other.name == "Player")
		{
			sRenderer.GetComponent<Renderer>().sortingOrder = 2;
		}
	}
}
