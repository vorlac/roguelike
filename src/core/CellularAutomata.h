#pragma once

#include <vector>

class CellularAutomata
    {
        private: 
            int CountNeighbors(int x, int y, int w, int h, std::vector<int> graph, int oob=1)
            {
                int neighbors = 0;
                // e
                if (x < w - 1)
                    neighbors += graph[x+1, y];    
                else
                    neighbors += oob;
                // se
                if (x < w-1 && y < h-1)
                    neighbors += graph[x+1, y+1];  
                else
                    neighbors += oob;
                // s
                if (y < h-1)
                    neighbors += graph[x, y+1];    
                else
                    neighbors += oob;
                // sw
                if (x > 0 && y < h-1)
                    neighbors += graph[x-1, y+1];
                else
                    neighbors += oob;
                // w
                if (x > 0)
                    neighbors += graph[x-1, y];
                else
                    neighbors += oob;
                // nw
                if (x > 0 && y > 0)
                    neighbors += graph[x-1, y-1];
                else
                    neighbors += oob;
                // n
                if (y > 0)
                    neighbors += graph[x, y-1];
                else
                    neighbors += oob;
                // ne
                if (y > 0 && x < w-1)
                    neighbors += graph[x+1, y-1];
                else
                    neighbors += oob;
                return neighbors;
            }

            std::vector<int> DoIteration(int w, int h, std::vector<int> graph)
            {
                std::vector<int> newGraph;
                for (int y=0; y<h; y++)
                for (int x=0; x<w; x++)
                {
                    bool alive = graph[x,y] == 1;
                    int neighbors = CountNeighbors(x, y, w, h, graph);

                    if (neighbors < 4)
                        newGraph[x,y] = 0;
                    else if (neighbors > 4)
                        newGraph[x,y] = 1;
                    else
                        newGraph[x,y] = 1;
                }
                return newGraph;
            }

            std::vector<int> Reverse(int w, int h, std::vector<int> graph)
            {
                std::vector<int> newGraph;
                for (int y=0; y<h; y++)
                for (int x=0; x<w; x++)
                {
                    if (graph[x,y] == 0)
                        newGraph[x,y] = 1;
                }
                return newGraph;
            }

        std::vector<int> GenerateNeighborCounts(int w, int h, std::vector<int> graph)
        {
            std::vector<int> newGraph;
            for (int y=0; y<h; y++)
            for (int x=0; x<w; x++)
            {
                if (graph[x,y] == 1)
                    newGraph[x,y] = CountNeighbors(x, y, w, h, graph, 0);
            }
            return newGraph;
        }

        std::vector<int> GenerateCellularAutomata(int w, int h, float prob, int iters) 
        {
            // var rand = new Random(1);

            std::vector<int> graph;
            
            for (int y=0; y<h; y++)
            for (int x=0; x<w; x++)
            {
                // if (rand.NextDouble() < prob)
                //     graph[x,y] = 1;
            }

            for(int i=0; i<iters; i++)
            {
                graph = DoIteration(w, h, graph);
            }

            return graph;
        }

        // std::vector<int> GenerateCellularAutomata(int w, int h, float prob, int iters, int diff=0)
        // {
        //     std::vector<int> mdGraph = GenerateCellularAutomata(w, h, prob, iters);
        //     mdGraph = CellularAutomata.Reverse(w, h, mdGraph);
        //     mdGraph = CellularAutomata.GenerateNeighborCounts(w, h, mdGraph);
        //     List<int> flatGraph = new List<int>();
        //     for (int y=0; y<h; y++)
        //     for (int x=0; x<w; x++)
        //         flatGraph.Add(mdGraph[x, y]);
        //     return flatGraph;
        // }
    };