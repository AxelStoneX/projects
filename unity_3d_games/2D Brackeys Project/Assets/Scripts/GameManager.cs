using UnityEngine;
using System.Collections;

public class GameManager : MonoBehaviour {
	 
	public static int playerScore1 = 0;
	public static int playerScore2 = 0;

	public GUISkin theSkin;

	Transform theBall;

	void Start()
	{
		theBall = GameObject.FindGameObjectWithTag("Ball").transform;
	}

	public static void Score (string wallName) 
	{
		if(wallName == "rightWall")
			playerScore1 += 1;
		else
			playerScore2 += 1;
	}

	void OnGUI()
	{
		GUI.skin = theSkin;
		GUI.Label(new Rect (Screen.width/2 - 150 - 18, 20, 100, 100), "" +playerScore1);
		GUI.Label(new Rect (Screen.width/2 + 150 - 18, 20, 100, 100), "" +playerScore2);

		if (GUI.Button (new Rect (Screen.width/2 - 121/2, 17, 121, 53 ), "RESET"))
		{
			playerScore1 = 0;
			playerScore2 = 0;

			theBall.gameObject.SendMessage("ResetBall");

		}
	}
}
