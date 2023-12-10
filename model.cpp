#include "model.h"
#include <Box2D/Box2D.h>
#include <QDebug>
#include <QApplication>

Model::Model(QObject *parent)
    : QObject{parent}, world(b2World(b2Vec2(0.0f, 1000.0f)))
{
    // Timer setup
    timer = new QTimer(this);
    connect (timer,
            &QTimer::timeout,
            this,
            &Model::UpdateView);

    world.SetContactListener(&contactListener);

    // Define the ground body.
    b2BodyDef groundBodyDef;

    // Set up the ground position. 90 seems to match the gameview's ground
    groundBodyDef.position.Set(0.0f, 100.0f);

    // Call the body factory which allocates memory for the ground body
    // from a pool and creates the ground box shape (also from a pool).
    // The body is also added to the world.
    b2Body* groundBody = world.CreateBody(&groundBodyDef);
    // Define the ground box shape.
    b2PolygonShape groundBox;
    // The extents are the half-widths of the box.
    groundBox.SetAsBox(500.0f, 2.0f);
    // Add the ground fixture to the ground body.
    groundBody->CreateFixture(&groundBox, 0.0f);

    b2BodyDef leftWallBodyDef;
    leftWallBodyDef.position.Set(-175.0f, 100.0f);
    b2Body* leftWallBody = world.CreateBody(&leftWallBodyDef);
    b2PolygonShape leftWallBox;
    leftWallBox.SetAsBox(2.0f, 800.0f);
    leftWallBody->CreateFixture(&leftWallBox, 0.0f);

    b2BodyDef rightWallBodyDef;
    rightWallBodyDef.position.Set(175.0f, 100.0f);
    b2Body* rightWallBody = world.CreateBody(&rightWallBodyDef);
    b2PolygonShape rightWallBox;
    rightWallBox.SetAsBox(2.0f, 800.0f);
    rightWallBody->CreateFixture(&rightWallBox, 0.0f);

    // Unsure if we need to add ground to bodies
    bodies.push_back(groundBody);

    // Create a repository for all potential Atoms in the game.
    for (unsigned int i = 0; i < 54; i++)
    {
        elementList.push_back(new Atom (nullptr, i+1));
    }

    for (Atom* atom : elementList){
        elementStatus[atom->elementNotation] = false;
    }
    bodies.push_back(leftWallBody);
    bodies.push_back(rightWallBody);

    numElementsFound = 0;
}

void Model::MakeCircleBody(float x, float y, float radius)
{
    b2BodyDef circleDef;
    circleDef.type = b2_dynamicBody;
    circleDef.position.Set(x, y);
    b2Body* circleBody = world.CreateBody(&circleDef);
    circleBody->SetUserData(this);

    b2CircleShape circleShape;
    circleShape.m_radius = radius;

    b2FixtureDef fixtureDef;
    fixtureDef.shape = &circleShape;
    fixtureDef.density = 1.0f;
    fixtureDef.friction = 0.3f;
    fixtureDef.restitution = 0.9f;
    circleBody->CreateFixture(&fixtureDef);
    bodies.push_back(circleBody);
    emit SendBodies(bodies);
}

void Model::BeginGame()
{
    timer->start(1000/60);
    emit SendStartGame();
    emit SendAtomList(elementList);
    emit SetStartButtonVisibility(false);
    emit SetQuitButtonVisibility(false);
    emit SetGameViewVisibility(true);
    emit SetGameOverLabelVisibility(false);
    emit SetTutorialButtonVisibility(false);
    emit SetTutorialButtonSideVisibility(true);
    emit SendStartGame();
}

void Model::UpdateView()
{
    world.Step(timeStep, velocityIterations, positionIterations);
    HandleCollision(world.GetContactList());
    emit UpdateWorld();
}

void Model::SendBodiesTemp()
{
    emit SendBodies(bodies);
    BeginGame();
}

void Model::SendElementStatus(QString element){
    if (!elementStatus[element]){
        emit RequestDisplayElementInfo(element);
        UpdateElementStatus(element);
    }
}

void Model::UpdateElementStatus(QString element){
    elementStatus[element] = true;
    numElementsFound +=1;
    emit RequestUpdateProgress(element, numElementsFound);
}

void Model::RecieveCheckForGameOver()
{
    for (unsigned int i = 3; i < bodies.size(); i++)
    {
        if (bodies[i]->GetPosition().y < bodies[1]->GetPosition().y - 400 && //This checks if the ball is above the red line.
            bodies[i]->GetLinearVelocity().y < 0) //This checks if the velocity is negative, it is it means the ball bounced on something which means it is not dropping from the top of the screen.
        {
            GameOver();
        }
    }
}

void Model::GameOver()
{
    for (unsigned int i = 3; i < bodies.size(); i++) //This behavior is temporary and we can change it to whatever we need later.
    {
        world.DestroyBody(bodies[i]);
    }

    vector<b2Body*> temp {bodies[0], bodies[1], bodies[2]};
    bodies = temp;
    emit SendBodies(bodies);

    emit SetGameOverLabelVisibility(true);
    emit SetGameViewVisibility(false);
    emit SetStartButtonVisibility(true);
    emit SetQuitButtonVisibility(true);
    emit SetTutorialButtonVisibility(true);
    emit SetTutorialButtonSideVisibility(false);
}

void Model::HandleCollision(b2Contact* collissions)
{
    b2Contact* currentContact = collissions;
    while(currentContact != nullptr)
    {
        b2Body* bodyA = currentContact->GetFixtureA()->GetBody();
        b2Body* bodyB = currentContact->GetFixtureB()->GetBody();

        // ignore contacts with the floor or walls
        if(bodyA->GetType() == 0 || bodyB->GetType() == 0)
            return;


        float radiusA = bodyA->GetFixtureList()->GetShape()->m_radius;
        float radiusB = bodyB->GetFixtureList()->GetShape()->m_radius;
        float newRadius = radiusA + radiusB;

        // check if one of them is a catalyst
        bool isACatalyst = (radiusA/3 >= 21 && radiusA/3 <= 30) || (radiusA/3 >= 39 && radiusA/3 <= 48);
        bool isBCatalyst = (radiusB/3 >= 21 && radiusB/3 <= 30) || (radiusB/3 >= 39 && radiusB/3 <= 48);

        QString elementA = elementList[radiusA/3-1]->elementNotation;
        QString elementB = elementList[radiusB/3-1]->elementNotation;
        // qDebug() << elementA << " is catalyst: "  << isACatalyst;
        // qDebug() << elementB << " is catalyst: " << isBCatalyst;


        bool isANobleGas = (radiusA/3 == 2) || (radiusA/3 == 10) || (radiusA/3 == 18) || (radiusA/3 == 36) || (radiusA/3 == 54);
        bool isBNobleGas = (radiusB/3 == 2) || (radiusB/3 == 10) || (radiusB/3 == 18) || (radiusB/3 == 36) || (radiusB/3 == 54);
        if(isANobleGas || isBNobleGas)
            return;

        // If non catalyst has a joint, ignore it
        // for ignoring elemets when joined to a catalyst
        if((isACatalyst && bodyB->GetJointList() != nullptr)
            || (isBCatalyst && bodyA->GetJointList() != nullptr)) {
            return;
        }

        // Two non catalysts collide and either one is joined, we don't want them to combine
        bool bothNonCatalyst = !isACatalyst && !isBCatalyst;
        bool eitherHaveJoints =  bodyB->GetJointList() != nullptr || bodyA->GetJointList() != nullptr;
        if(bothNonCatalyst && eitherHaveJoints) {
            return;
        }


        // Prevents catalysts from reacting
        if((!isACatalyst && isBCatalyst) || (isACatalyst && !isBCatalyst))
        {
            // join the bodies and calculate the catalyst threshold
            if(isACatalyst)
            {
                if(bodyB->GetJointList() == nullptr) {
                    Catalyze(bodyA, bodyB);
                }
            }
            else
            {
                if(bodyA->GetJointList() == nullptr) {
                    Catalyze(bodyB, bodyA);
                }
            }
        }
        else if(newRadius/3-1 < 54)
        {
            qDebug() << elementA << " combine " << elementB;
            // Make a circle based on posA
            MakeCircleBody(bodyA->GetPosition().x, bodyA->GetPosition().y, newRadius);
            // Remove bodies when they collide
            RemoveBodies(bodyA);
            RemoveBodies(bodyB);

            // check to see if element is new
            SendElementStatus(elementList[newRadius/3-1]->elementNotation);
        }
        currentContact = currentContact->GetNext();
    }
}

void Model::Catalyze(b2Body* catalyst, b2Body* nonCatalyst)
{
    JoinBodies(catalyst, nonCatalyst);
    b2Vec2 catalystPos = catalyst->GetPosition();
    float radius = catalyst->GetFixtureList()->GetShape()->m_radius;
    int catalystThreshold = radius/12;
    qDebug() << "catalyst threshold: " << catalystThreshold;
    int newRadius = 0;

    // When reaction occurs
    if(joinedBodies[catalyst].size() >= (unsigned int) catalystThreshold)
    {
        qDebug() << "Removing catalyst";
        // Crashing inconsistenly when reaction occurs.
        // Probably something to do with the order that bodies and joints are deleted

        // add new bodies
//        for(b2Body* body : joinedBodies[catalyst]) {
//            MakeCircleBody(catalystPos.x, catalystPos.y, body->GetFixtureList()->GetShape()->m_radius);
//        }


          // recombining crashes
//        // remove the joints attatched to the catalyst
//        int buffer = 2000;
//        b2JointEdge* currentJoint = catalyst->GetJointList();
//        while(currentJoint != nullptr)
//        {
//            QTimer::singleShot(buffer, this, [=](){world.DestroyJoint(currentJoint->joint);});
//            currentJoint = currentJoint->next;
//            buffer = buffer + 2000;
//        }
//        // Remove joined bodies
        for(b2Body* body : joinedBodies[catalyst]) {
            newRadius += body->GetFixtureList()->GetShape()->m_radius;
            RemoveBodies(body);
        }
        RemoveBodies(catalyst);

        if(newRadius/3-1 >54) {
            newRadius = 162;
        }

        MakeCircleBody(0, 0, newRadius);
        // We are not deleting every joined noncatalyst, just the most recently joined.
        // Thus the non removed noncatayst isn't removed from the jointCount.

        // Can fix by changing jointCount to just hold joints (map of catalyst and vector of non catalysts)
    }
}

void Model::RemoveBodies(b2Body* body)
{
    // Remove bodyA from bodies and world
    vector<b2Body*>::iterator it = std::find(bodies.begin(), bodies.end(), body);
    bodies.erase(it);
    world.DestroyBody(body);
    // catalystJointCount.erase(body);
    joinedBodies.erase(body);

    emit SendBodies(bodies);
}

void Model::JoinBodies(b2Body* bodyA, b2Body* bodyB)
{
    qDebug() << "begin joint creation";
    float radiusA = bodyA->GetFixtureList()->GetShape()->m_radius;
    float radiusB = bodyB->GetFixtureList()->GetShape()->m_radius;

    // Create a joint, attaching bodies
    b2RopeJointDef jointDef;
    jointDef.bodyA = bodyA;
    jointDef.bodyB = bodyB;
    jointDef.localAnchorA = b2Vec2 (bodyA->GetLocalCenter().x,bodyA->GetLocalCenter().y + joinedBodies[bodyA].size());
    jointDef.localAnchorB = bodyB->GetLocalCenter();
    jointDef.maxLength = radiusA + radiusB + 5;
    jointDef.collideConnected = true;
    bodyA->GetWorld()->CreateJoint(&jointDef);

    // Accumlate joint count
    // catalystJointCount[bodyA] += 1;
    // catalystJointCount[bodyB] += 1;

    // Keep track of which bodies are joined
    joinedBodies[bodyA].push_back(bodyB);
    joinedBodies[bodyB].push_back(bodyA);


    // qDebug() << "catalyst joint count: " << catalystJointCount[bodyA];
    // qDebug() << "non catalyst joint count: " << catalystJointCount[bodyB];

    qDebug() << "catalyst joint count: " << joinedBodies[bodyA].size();
    qDebug() << "non catalyst joint count: " << joinedBodies[bodyB].size();

}

void Model::QuitGame() {
    QApplication::quit();
}

void Model::OpenTutorial()
{

    emit SetTutorialViewVisability(true);
    //emit SetQuitButtonVisibility(false);
    //emit SetStartButtonVisibility(false);
}
