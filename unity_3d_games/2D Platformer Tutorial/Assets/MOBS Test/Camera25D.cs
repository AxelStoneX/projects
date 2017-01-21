using UnityEngine;
using System.Collections;

public class Camera25D : MonoBehaviour {

	public Transform target;
	public Transform background;


	private bool outOfBorderX;
	private bool outOfBorderY;
	private float offsetX;
	private float offsetY;
	private Vector3 camTarget;
	private float camWidth;
	private float camHeight;
	private float backgroundWidth;
	private float backgroundHeight;
	
	void Start () {

		backgroundWidth = background.GetComponent<SpriteRenderer>().sprite.bounds.extents.x - background.transform.position.x;
		backgroundHeight = background.GetComponent<SpriteRenderer>().sprite.bounds.extents.y - background.transform.position.y;
		camWidth = Mathf.Abs (transform.position.x - Camera.main.ScreenToWorldPoint (new Vector3 (Screen.width, 0, 0)).x);
		camHeight = Mathf.Abs (transform.position.y - Camera.main.ScreenToWorldPoint (new Vector3 (0, Screen.height, 0)).y);
		offsetX = backgroundWidth - camWidth;
		offsetY = backgroundHeight - camHeight;
		camTarget = CamTarget();
		transform.position = camTarget;
	}

	void Update () {

		camTarget = CamTarget();
		transform.position = camTarget;

	}

	Vector3 CamTarget()
	{
		//This block can be deleted for performance
		backgroundWidth = background.GetComponent<SpriteRenderer>().sprite.bounds.extents.x - background.transform.position.x;
		backgroundHeight = background.GetComponent<SpriteRenderer>().sprite.bounds.extents.y - background.transform.position.y;
		camWidth = Mathf.Abs (transform.position.x - Camera.main.ScreenToWorldPoint (new Vector3 (Screen.width, 0, 0)).x);
		camHeight = Mathf.Abs (transform.position.y - Camera.main.ScreenToWorldPoint (new Vector3 (0, Screen.height, 0)).y);
		offsetX = backgroundWidth - camWidth;
		offsetY = backgroundHeight - camHeight;
		//End of the deletable block

		outOfBorderX = (target.position.x <= -offsetX || target.position.x >= offsetX);
		outOfBorderY = (target.position.y <= -offsetY || target.position.y >= offsetY);
		if (!outOfBorderX && !outOfBorderY)
			camTarget = new Vector3 (target.position.x, target.position.y, transform.position.z);
		else
		{
			if (outOfBorderX && !outOfBorderY)
				camTarget = new Vector3 (Mathf.Sign(transform.position.x) * offsetX, target.position.y, transform.position.z );
			if (!outOfBorderX && outOfBorderY)
				camTarget = new Vector3 (target.position.x, Mathf.Sign(transform.position.y) * offsetY, transform.position.z );
			if (outOfBorderX && outOfBorderY)
				camTarget = new Vector3 (Mathf.Sign(transform.position.x) * offsetX, Mathf.Sign(transform.position.y) * offsetY, transform.position.z );
		}
		return camTarget;
	}
}
