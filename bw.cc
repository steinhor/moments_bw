#include "canonical.h"
#include "MuBOverT.cc"
using namespace std;

int main(int argc,char *argv[]){
	if(argc!=2){
		printf("Usage: bw parameter_filename\n");
		exit(1);
	}
	const int NROOTS=7;
	int ievent,nevents,i,b0,q0,s0=0,iroots; // initial charge, baryon no. and strangeness
	double Omega,muBoverT,ff,ffa=1.0;
	double roots[NROOTS]={7.7,11.5,19.6,27.0,39.0,62.4,200.0};
	// These correspond to rhoH=0.333333333
	double T[NROOTS]={143.45,149.53,153.4,154.45,155.08,155.44,155.66};
	double rhoB[NROOTS]={0.143511,0.0942362,0.0525105,0.0373777,0.0255775,0.0159063,0.00497411};
	//
	parameterMap parmap;
	parameter::ReadParsFromFile(parmap,string(argv[1]));
	Cacceptance acceptance(parameter::getD(parmap,"ACCEPTANCE",0.25));
	Cmoments moments(&acceptance);
	vector<CResInfo *> resinfovec;
	vector<Cpart> partvec;
	CpartitionFunction pf(parmap);
	pf.DECAY=false;  // don't decay particles (will be done by blastwave)
	CblastWave blastwave(parmap,pf.randy,pf.reslist);
	//pf.randy->reset(-time(NULL));
	pf.randy->reset(1234);
	//pf.WriteZ();
	//pf.ReadZ();

	nevents=parameter::getI(parmap,"NEVENTS",100);
	Omega=parameter::getD(parmap,"OMEGA",100);
	string file=parameter::getS(parmap,"MOMENTS_OUTPUT_FILE","moments.dat");
	pf.SetOmega(Omega);

	for(iroots=0;iroots<NROOTS;iroots++){
		printf("____ iroots=%d ____\n",iroots);
		muBoverT=GetMuBOverT(roots[iroots]);
		ff=log(1.0+ffa*(roots[iroots]-roots[0])/(roots[NROOTS-1]-roots[0]))/log(1.0+ffa); // interpolating weight from zero to 1
		blastwave.Tf=120.0-20*ff;
		blastwave.uperpx=0.5+(0.74-0.5)*ff;
		blastwave.uperpy=blastwave.uperpx;
		pf.T=T[iroots];
		pf.CalcZ();
		printf("----- Z calculated\n");
		for(ievent=0;ievent<nevents;ievent++){
			do{
				b0=pf.randy->GetNPoissonian(rhoB[iroots]*Omega);
				q0=pf.randy->GetNPoissonian(0.5*rhoB[iroots]*Omega);
				if(!pf.CheckRelevance(pf.NhadMAX/2,b0,q0,s0)){
					printf("picked b0=%d or q0=%d out of bounds\n",b0,q0);
					printf("If this happens often, increase pf.NhadMAX\n");
				}
			}while(!pf.CheckRelevance(pf.NhadMAX/2,b0,q0,s0));
			pf.GenEvent(b0,q0,s0,resinfovec);
			blastwave.GenerateParts(resinfovec,partvec);
			moments.IncrementMoments(partvec);
			partvec.clear();
			if((ievent+1)%(nevents/10)==0)
				printf("Finished %g percent\n",100.0*(ievent+1.0)/double(nevents));
		}
		moments.Summarize(file,Omega,rhoB[iroots],0.5*rhoB[iroots],roots[iroots],T[iroots]);
		moments.Clear();

	}

	return 0;
}