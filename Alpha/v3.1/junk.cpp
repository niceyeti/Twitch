  trigger = 0;
  for(i = 0; i < (inData.size() - segmentWidth - 1); i += sampleRate){
    //ignore points outside of the active region, including the <start/stop> region
    if(InBounds(inData[i]) && InBounds(inData[i+segmentWidth-1])){
      //measures absolute change in distance, a dumb attribute
      //lastDist = dist;
      //dist = layoutManager->DoubleDistance(inData[i],inData[i+segmentWidth-1]);
      //dDist = dist - lastDist;
      //dydx = layoutManager->DyDx(inData[i],inData[i+segmentWidth-1]); //slope
      //dydx = layoutManager->AvgDyDx(inData,i,segmentWidth);
      //avgDist = layoutManager->AvgDistance(inData,i,segmentWidth);
      //measures clustering of a group of points, but only the points themselves
      //coVar = layoutManager->CoVariance(inData,i,segmentWidth);
      stDev = layoutManager->CoStdDeviation(inData,i,segmentWidth); //(sigmaX*sigmaY)^2
	    //lastTheta = avgTheta;
	    //avgTheta = layoutManager->AvgTheta(inData,i,segmentWidth);  //get the avg direction for a sequence of points
	    //dTheta = (avgTheta - lastTheta) / 2.0;
      //printf("stDev,avgDist,dDist,dydx,avgTheta,dTheta: %9.3f  %9.3f  %9.3f  %9.3f  %9.3f  %9.3f\n",stDev,avgDist,dDist,dydx,avgTheta,dTheta);

      /*
        Implements an event oriented state machine in which it is easier to exit states than enter them.
        This helps detect the confounded edge between neighbor-key transitions, which is a difficult problem.
      */
      if(stDev > stDev_HardTrigger){
        trigger++;

        //trigger and collect event
        if(trigger > triggerCount){
          currentAlpha = nearestAlpha = layoutManager->GetNearestKey(inData[i]);
          trigger = 0;

          eventStart = i;
          i++;
          stDev = layoutManager->CoStdDeviation(inData,i,segmentWidth);
          //hold state, unless there is a stDev change and an alpha change. Only if both alpha changes and stDev throws, will we exit.
          // A new event will immediately be thrown to catch the neighbor key event.
          while(i < inData.size() && (stDev < stDev_SoftTrigger || nearestAlpha == currentAlpha)){
            stDev = layoutManager->CoStdDeviation(inData,i,segmentWidth);
            currentAlpha = layoutManager->GetNearestKey(inData[i]);
            printf("pt. stDev, avgDist, dist, dx, avgTheta, dTheta: %9.3f\n",stDev);
            i++;
          }
          //exit state either by hard/fast exit, or soft-exit to an adjacent key
          eventEnd = i;
          i--;

          //capture event data and push it, then continue monitoring for new ones. Event is pushed only if unique from the last one.
          PointMu outPoint;
          outPoint.ticks = eventEnd - eventStart + hardThreshold;
          CalculateMean(eventStart,eventEnd,inData,outPoint);
          outPoint.alpha = layoutManager->FindNearestKey(outPoint.pt);
          cout << "hit mean for " << outPoint.alpha << endl;

          //NOTE A new cluster is appended only if it is a unique letter; this prevents repeated chars.
          if(midData.empty()){  //this is just an exception check, so we don't deref a -1 index in the next if-stmt, when the vec is empty
            midData.push_back(outPoint);
          }
          //verify incoming alpha cluster is unique from previous alpha
          else if(outPoint.alpha != midData[midData.size()-1].alpha){
            midData.push_back(outPoint);
          }
        }
      }

