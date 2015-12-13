/* --std=c90 -Wv */
void f1(int t) {
    if (t)
      while(t)
        for (; t;)
          switch(t)
            { case 1:
              do
                {
                  {
                    {
                      for(; t; )
                        if (!t)
                          {
                            {
                              {
                                {
                                  for (; t; )
                                      f1(2);    /* warning */
                                  {
                                    f1(2);    /* warning */
                                  }
                                  while (t)
                                    while (t)    /* warning */
                                      ;
                                }
                              }
                            }
                          }
                    }
                  }
                } while(t);
            }
}
