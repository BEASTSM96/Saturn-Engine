#include "__FILE_NAME__.h"

__FILE_NAME__::__FILE_NAME__()
{

}

__FILE_NAME__::~__FILE_NAME__()
{

}

// Called Once, at the start of runtime.
void __FILE_NAME__::BeginPlay()
{
	Super::BeginPlay();
}

// Called every frame.
void __FILE_NAME__::OnUpdate( Saturn::Timestep ts )
{
	Super::OnUpdate( ts );
}

// Called every frame (fixed timestep).
void __FILE_NAME__::OnPhysicsUpdate( Saturn::Timestep ts )
{
	Super::OnUpdate( ts );
}

void __FILE_NAME__::SetupInputBindings()
{

}
