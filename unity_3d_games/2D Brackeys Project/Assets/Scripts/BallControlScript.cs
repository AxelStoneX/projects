using UnityEngine;
using System.Collections;

public class BallControlScript : MonoBehaviour {

	float randomNumber;
	public float ballStartSpeed = 100;

	void Update ()
	{
		float velX = GetComponent<Rigidbody2D>().velocity.x;
		float velY = GetComponent<Rigidbody2D>().velocity.y;
		if (velX < 12 && velX > -12 && velX != 0)
		{
			if( velX > 0 )
				GetComponent<Rigidbody2D>().velocity = new Vector2 (14, velY);
		    else
				GetComponent<Rigidbody2D>().velocity = new Vector2 (-14, velY);
		}
	}
	
	IEnumerator Start () 
	{
		yield return new WaitForSeconds (1.5f);
		GoBall ();
	}

	void GoBall()
	{
		randomNumber = Random.Range (0, 2);
		if (randomNumber <= 0.5) 
			GetComponent<Rigidbody2D>().AddForce (new Vector2 (ballStartSpeed,10));
		else 
			GetComponent<Rigidbody2D>().AddForce (new Vector2 (-ballStartSpeed,-10));
	}

	IEnumerator ResetBall()
	{
		GetComponent<Rigidbody2D>().velocity = new Vector2 (0, 0);
		transform.position = new Vector2 (0, 0);

		yield return new WaitForSeconds (1f);
		GoBall ();
	}

	void OnCollisionEnter2D (Collision2D colInfo)
	{
		if (colInfo.collider.tag == "Player")
		{
			float velX = GetComponent<Rigidbody2D>().velocity.x;
			float velY = GetComponent<Rigidbody2D>().velocity.y;
			velY = velY + colInfo.collider.GetComponent<Rigidbody2D>().velocity.y;
			GetComponent<Rigidbody2D>().velocity = new Vector2 (velX, velY);
			GetComponent<AudioSource>().pitch = Random.Range (0.8f, 1.2f);
			GetComponent<AudioSource>().Play ();
		}
	
	}
}
