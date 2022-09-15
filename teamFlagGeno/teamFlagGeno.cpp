/*
 Team Flag Geno.
 Holding your own team flag acts as Genocide.
 
 Server Variables:
 _genoPotatoRatio - the required wins to losses ratio a player must have to be eligible for geno
 _genoCooldownTime - the amount of time a team must wait between genocide.
 
 Extra notes:
 - This plugin works in parallel with the avengerFlag plugin, but is not needed
   for this plugin to work. Avenger reverses the effect of geno back on their own team.
 
 ./configure --enable-custom-plugins=teamFlagGeno
*/

#include "bzfsAPI.h"
#include <math.h>
using namespace std;


class TeamFlagGeno : public bz_Plugin
{
	virtual const char* Name()
	{
		return "Team Flag Geno";
	}

	virtual void Init(const char*);
	virtual void Event(bz_EventData*);
	~TeamFlagGeno();

	virtual void Cleanup(void)
	{
		Flush();
	}
	
	std::map<bz_eTeamType, double> lastGenoTime;		// Remembers the last time the team commited genocide.
	double redTeamLastGeno;
	double greenTeamLastGeno;
};

BZ_PLUGIN(TeamFlagGeno)

void TeamFlagGeno::Init (const char*)
{
	bz_registerCustomBZDBDouble("_genoPotatoRatio", 0.3);		// Wins to losses ratio
	bz_registerCustomBZDBInt("_genoCooldownTime", 20);
    Register(bz_ePlayerDieEvent);
    Register(bz_eFlagGrabbedEvent);
}

TeamFlagGeno::~TeamFlagGeno() {}

void grabbedOwnTeamFlag(data->playerID)
{
	if (bz_getPlayerTeam(data->playerID) != eRedTeam)
	{
		bz_sendTextMessagef(BZ_SERVER, eRedTeam, "Watch out! %s is packing genocide!", bz_getPlayerCallsign(data->playerID).c_str());
	}
	if (bz_getPlayerTeam(data->playerID) != eGreenTeam)
	{
		bz_sendTextMessagef(BZ_SERVER, eGreenTeam, "Watch out! %s is packing genocide!", bz_getPlayerCallsign(data->playerID).c_str());
	}
	if (bz_getPlayerTeam(data->playerID) != ePurpleTeam)
	{
		bz_sendTextMessagef(BZ_SERVER, ePurpleTeam, "Watch out! %s is packing genocide!", bz_getPlayerCallsign(data->playerID).c_str());
	}
	if (bz_getPlayerTeam(data->playerID) != eBlueTeam)
	{
		bz_sendTextMessagef(BZ_SERVER, eBlueTeam, "Watch out! %s is packing genocide!", bz_getPlayerCallsign(data->playerID).c_str());
	}
}

void TeamFlagGeno::Event(bz_EventData *eventData)
{
	switch (eventData->eventType)
	{
		// This part does the whole "Watch out, so and so has geno..." when someone
		// grabs geno.
		case (bz_eFlagGrabbedEvent):
		{
			bz_FlagGrabbedEventData_V1 *data = (bz_FlagGrabbedEventData_V1*) eventData;
			bz_ApiString flag = bz_getFlagName(data->flagID);
			// If the red team or the green team is holding their own team flag,
			// then send a server message to other team warning them.
			if ((bz_getPlayerTeam(data->playerID) == eGreenTeam && flag == "G*") ||
				(bz_getPlayerTeam(data->playerID) == eRedTeam && flag == "R*"))
			{
				grabbedOwnTeamFlag(data->playerID);
			}
		} break;
		case (bz_ePlayerDieEvent):
	 	{
	   		bz_PlayerDieEventData_V2 *data = (bz_PlayerDieEventData_V2 *) eventData;
	   		
	   		int killerID = data->killerID;
			bz_ApiString flagHeldWhenKilled = bz_getFlagName(data->flagHeldWhenKilled);
	   		
	   		// Handles player's world weapon shots
			if (data->killerID == BZ_SERVER)
			{
				uint32_t shotGUID = bz_getShotGUID(data->killerID, data->shotID);
				
				if (bz_shotHasMetaData(shotGUID, "owner"))
					killerID = bz_getShotMetaDataI(shotGUID, "owner");
			}
			
			// Award the killer bounty points if we die with team flag
			if (data->team != data->killerTeam && (flagHeldWhenKilled == "G*" || flagHeldWhenKilled == "R*"))
			{
				if (flagHeldWhenKilled == "G*")
					bz_sendTextMessage(BZ_SERVER, killerID, "Shooting the Green Team carrier earned you 2 extra bounty points.");
				else if (flagHeldWhenKilled == "R*")
					bz_sendTextMessage(BZ_SERVER, killerID, "Shooting the Red Team carrier earned you 2 extra bounty points.");
				data->flagHeldWhenKilled = -1;
				bz_incrementPlayerWins(killerID, 2);
			}
					
			// If shot by a player holding their own team flag... (aka Geno)
			if ((data->flagKilledWith == "R*" && data->killerTeam == eRedTeam)
				|| (data->flagKilledWith == "G*" && data->killerTeam == eGreenTeam)
				 || (data->flagKilledWith == "P*" && data->killerTeam == ePurpleTeam)
				  || (data->flagKilledWith == "B*" && data->killerTeam == eBlueTeam))
			{
	 			// Don't cover the case of shooting yourself
	 			if (data->playerID == killerID)
	 				return;   	
				
				// This grabs a list of all the players on the server.
				bz_APIIntList* playerList = bz_getPlayerIndexList();
				
				// If they're holding the Avenger flag, blow the other team up instead!
				if (flagHeldWhenKilled == "AV")
				{
					bz_sendTextMessagef(BZ_SERVER, BZ_ALLUSERS, "AVENGED! %s's death has been avenged, having %s destroyed with genocide instead!", bz_getPlayerCallsign(data->playerID), bz_getPlayerCallsign(killerID));
					
					int teamKills = 0;
					
					for (int i = 0; i < bz_getPlayerCount(); i++)
					{
						if (bz_getPlayerTeam(playerList->get(i)) == data->killerTeam)
						{
							bz_BasePlayerRecord* playerRecord = bz_getPlayerByIndex(playerList->get(i));
							
							// If the player is alive, kill them.
							if (playerRecord && playerRecord->spawned)
							{
								bz_killPlayer(playerList->get(i), false, data->playerID, "G");
								// bz_killPlayer automatically decrements their score, so
								// we add it back by the following.
								bz_incrementPlayerLosses(playerList->get(i), -1);
								teamKills++;
								
								// Don't send this message to the person who got hit with avenged genocide
								if (playerList->get(i) != killerID)
									bz_sendTextMessagef(playerList->get(i), playerList->get(i), "Teammate %s got hit with genocide by %s. Don't mess with the avengers.", bz_getPlayerCallsign(killerID), bz_getPlayerCallsign(data->playerID));
							}
							
							// Free the record.
							bz_freePlayerRecord(playerRecord);
						}
					}
					
					// Make the geno victim lose the points for each of his teammates
					// whose deaths he is responsible for.
					// Remember: the KILLER is the victim with avenged geno.
					bz_incrementPlayerLosses(data->killerID, teamKills);
				}
				else
				{
					// If they're a potato, don't geno.
					if (bz_getPlayerLosses(data->playerID) != 0)
					{
						if ((double)bz_getPlayerWins(data->playerID)/(double)bz_getPlayerLosses(data->playerID) <= bz_getBZDBDouble("_genoPotatoRatio"))
						{
							bz_sendTextMessagef(BZ_SERVER, killerID, "%s is a potato, not eligible for geno.", bz_getPlayerCallsign(data->playerID));
							return;
						}
					}
					
					
					if (bz_getCurrentTime() - lastGenoTime[data->killerTeam] <= bz_getBZDBInt("_genoCooldownTime"))
					{
						int timePassedSinceGeno = (int)(bz_getCurrentTime() - lastGenoTime[data->killerTeam]);
						
						bz_sendTextMessagef(BZ_SERVER, data->killerID, "Your team recently commited genocide within the last %d seconds.", bz_getBZDBInt("_genoCooldownTime"));
						bz_sendTextMessagef(BZ_SERVER, data->killerID, "Please wait another %d seconds to commit genocide again.", bz_getBZDBInt("_genoCooldownTime") - timePassedSinceGeno);
						
						return;
					}
					
					bz_sendTextMessagef(BZ_SERVER, BZ_ALLUSERS, "OUCH! %s just got hit with genocide!", bz_getPlayerCallsign(data->playerID));
					// Sets the last time geno happened to now. Used for cooldown.
					lastGenoTime[data->killerTeam] = bz_getCurrentTime();
					
					// This variable keeps track of how many of the geno victim's
					// teammates get killed due to the player getting nailed with
					// geno (including themselves). So, player A has 10 teammates, but only
					// 5 of them are spawned, they should only lose 6 points, one for each
					// of the teammates and one for themselves.
					int teamKills = 0;

					for (int i = 0; i < bz_getPlayerCount(); i++)
					{
						// If the player is on the geno victim's team, kill them if they are alive.
						if (bz_getPlayerTeam(playerList->get(i)) == data->team)
						{
							bz_BasePlayerRecord* playerRecord = bz_getPlayerByIndex(playerList->get(i));
							
							// If the player is alive, kill them.
							if (playerRecord && playerRecord->spawned)
							{
								bz_killPlayer(playerList->get(i), false, killerID, "G");
								// bz_killPlayer automatically decrements their score, so
								// we add it back by the following.
								bz_incrementPlayerLosses(playerList->get(i), -1);
								teamKills++;
								
								// Don't send this message to the person who got hit with genocide
								if (playerList->get(i) != data->playerID)
									bz_sendTextMessagef(playerList->get(i), playerList->get(i), "Teammate %s got shot with genocide by %s", bz_getPlayerCallsign(data->playerID), bz_getPlayerCallsign(killerID));
							}
							
							// Free the record.
							bz_freePlayerRecord(playerRecord);
						}
					}
					
					// Make the geno victim lose the points for each of his teammates
					// whose deaths he is responsible for.
					bz_incrementPlayerLosses(data->playerID, teamKills);
				}
			}
	 	} break;
	 	default:
	 		break;
	}
}
