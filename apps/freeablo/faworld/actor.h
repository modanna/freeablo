#ifndef ACTOR_H
#define ACTOR_H

#include <map>

#include <boost/format.hpp>

#include <statemachine/statemachine.h>
#include <misc/misc.h>

#include "../engine/net/netmanager.h"
#include "../farender/animationplayer.h"
#include "../fasavegame/savegame.h"

#include "position.h"
#include "gamelevel.h"
#include "world.h"
#include "faction.h"
#include "movementhandler.h"
#include "actoranimationmanager.h"


namespace Engine
{
    class NetManager;
    class Server;
    class Client;
}

namespace FAWorld
{
    class ActorStats;
    class Behaviour;
    class World;

    namespace ActorState
    {
        class BaseState;
    }

    class Actor : public NetObject
    {
        STATIC_HANDLE_NET_OBJECT_IN_CLASS()

        friend class ActorState::BaseState; // TODO: fix

        public:
            Actor(const std::string& walkAnimPath="",
                  const std::string& idleAnimPath="",
                  const std::string& dieAnimPath=""
                  );

            virtual void update(bool noclip);
            virtual ~Actor();
            virtual std::string getDieWav(){return "";}
            virtual std::string getHitWav(){return "";}
            virtual void setSpriteClass(std::string className){UNUSED_PARAM(className);}
            virtual void takeDamage(double amount);
            virtual int32_t getCurrentHP();
            bool isAttacking = false;
            bool isTalking = false;

            StateMachine::StateMachine<Actor>* mActorStateMachine;

            ActorAnimationManager& getAnimationManager()
            {
                return mAnimation;
            }

            bool isPassable()
            {
                return mPassable || mIsDead;
            }

            void setPassable(bool passable)
            {
                mPassable = passable;
            }
            
            int32_t getId()
            {
                return mId;
            }

            void teleport(GameLevel* level, Position pos);
            GameLevel* getLevel();

            virtual bool attack(Actor * enemy)
            {
                UNUSED_PARAM(enemy);
                return false;
            }

            virtual bool talk(Actor * actor)
            {
                UNUSED_PARAM(actor);
                return false;
            }

            MovementHandler mMoveHandler;
            Actor* actorTarget = nullptr;

            Position getPos() const
            {
                return mMoveHandler.getCurrentPosition();
            }

        //private: //TODO: fix this
           

            virtual void die();
            
            bool canTalk() const;
            bool isDead() const;
            bool isEnemy(Actor* other) const;
            std::string getActorId() const;
            void setActorId(const std::string& id);
            void setCanTalk(bool canTalk);

            bool canWalkTo(int32_t x, int32_t y);

            void attachBehaviour(Behaviour* behaviour) {
              mBehaviour = behaviour;
            };

            ActorStats * mStats=nullptr;

            template <class Stream>
            Serial::Error::Error faSerial(Stream& stream)
            {
                auto destTmp = mMoveHandler.getDestination();
                auto levelTmp = mMoveHandler.getLevel();
                serialise_object(stream, mMoveHandler);

                
                if (!stream.isWriting())
                {
                    // make sure we let the level object know if we've changed levels
                    if (levelTmp != mMoveHandler.getLevel())
                    {
                        if (levelTmp)
                            levelTmp->removeActor(this);

                        mMoveHandler.getLevel()->addActor(this);

                        destTmp = mMoveHandler.getCurrentPosition().current(); // just sit still after a level change
                    }


                    // don't sync our own destination, we set that ourselves
                    if ((Actor*)World::get()->getCurrentPlayer() == this)
                        mMoveHandler.setDestination(destTmp);
                }

                serialise_object(stream, mAnimation);

                //serialise_object(stream, mPos);
                //serialise_int(stream, 0, 2048, mFrame);
                //serialise_enum(stream, AnimState::AnimState, mAnimState);




                //int32_t destXTmp = mDestination.first;
                //int32_t destYTmp = mDestination.second;
               /* int32_t levelIndexTmp = -1;
                if (mLevel)
                    levelIndexTmp = mLevel->getLevelIndex();*/


                //serialise_int32(stream, destXTmp);
                //serialise_int32(stream, destYTmp);
                //serialise_int32(stream, levelIndexTmp);


                //if (!stream.isWriting())
                //{
                //    if ((Actor*)World::get()->getCurrentPlayer() != this)
                //    {
                //        // don't want to read destination for our player object,
                //        // we keep track of our own destination
                //        mDestination.first = destXTmp;
                //        mDestination.second = destYTmp;

                //        if (levelIndexTmp != -1)
                //            setLevel(World::get()->getLevel(levelIndexTmp));
                //    }
                //}

                /*uint32_t walkAnimIndex = 0;
                uint32_t idleAnimIndex = 0;
                uint32_t dieAnimIndex = 0;
                uint32_t attackAnimIndex = 0;
                uint32_t hitAnimIndex = 0;

                if (stream.isWriting()) 
                {
                    if (mWalkAnim)
                        walkAnimIndex = mWalkAnim->getCacheIndex();
                    if (mIdleAnim)
                        idleAnimIndex = mIdleAnim->getCacheIndex();
                    if (mDieAnim)
                        dieAnimIndex = mDieAnim->getCacheIndex();
                    if (mAttackAnim)
                        attackAnimIndex = mAttackAnim->getCacheIndex();
                    if (mHitAnim)
                        hitAnimIndex = mHitAnim->getCacheIndex();
                }

                serialise_int32(stream, walkAnimIndex);
                serialise_int32(stream, idleAnimIndex);
                serialise_int32(stream, dieAnimIndex);
                serialise_int32(stream, attackAnimIndex);
                serialise_int32(stream, hitAnimIndex);

                if (!stream.isWriting())
                {
                    mWalkAnim = FARender::getDefaultSprite();
                    mIdleAnim = FARender::getDefaultSprite();
                    mDieAnim = FARender::getDefaultSprite();
                    mAttackAnim = FARender::getDefaultSprite();
                    mHitAnim = FARender::getDefaultSprite();

                    auto netManager = Engine::NetManager::get();

                    if (walkAnimIndex)
                        mWalkAnim = netManager->getServerSprite(walkAnimIndex);
                    if (idleAnimIndex)
                        mIdleAnim = netManager->getServerSprite(idleAnimIndex);
                    if (dieAnimIndex)
                        mDieAnim = netManager->getServerSprite(dieAnimIndex);
                    if (attackAnimIndex)
                        mAttackAnim = netManager->getServerSprite(attackAnimIndex);
                    if (hitAnimIndex)
                        mHitAnim = netManager->getServerSprite(hitAnimIndex);
                }*/

                serialise_bool(stream, mIsDead);

                if(mStats)
                    serialise_object(stream, *mStats);

                return Serial::Error::Success;
            }

        protected:
            Behaviour* mBehaviour = nullptr;

            bool mIsDead = false;
            bool mCanTalk = false;
            Faction mFaction;

            bool mPassable = false;

            ActorAnimationManager mAnimation;

            friend class boost::serialization::access;

            template<class Archive>
            void save(Archive & ar, const unsigned int version) const
            {
                UNUSED_PARAM(version);
                UNUSED_PARAM(ar);
                assert(false);
                //ar & this->mPos;
                //ar & this->mFrame;
                //ar & this->mDestination;
            }

            template<class Archive>
            void load(Archive & ar, const unsigned int version)
            {
                UNUSED_PARAM(version);
                UNUSED_PARAM(ar);
                assert(false);
                //ar & this->mPos;
                //ar & this->mFrame;
                //ar & this->mDestination;
            }

            virtual bool canIAttack(Actor * actor);
            bool canTalkTo(Actor * actor);

            BOOST_SERIALIZATION_SPLIT_MEMBER()

        private:
            std::string mActorId;
            int32_t mId;
            friend class Engine::Server; // TODO: fix
            friend class Engine::Client;
            friend class Engine::NetManager;
    };
}

#endif
