import pandas as pd
import matplotlib.pyplot as plt
import seaborn as sns

def main():
    try:
        df = pd.read_csv("benchmark_results.csv")
    except Exception as e:
        print(f"Error reading benchmark_results.csv: {e}")
        return
    
    sns.set_theme(style="whitegrid")
    plt.figure(figsize=(10, 6))
    
    methods = df['Method'].unique()
    colors = sns.color_palette("husl", len(methods))
    markers = {'Single': 'o', 'Mutex': 's', 'Atomic': '^'}
    
    for idx, method in enumerate(methods):
        method_df = df[df['Method'] == method].sort_values('Elements')
        
        lbl = f'{method} (1 thread)' if method == 'Single' else f'{method} (max logical threads)'
        
        plt.plot(method_df['Elements'], method_df['TimeMs'], 
                 marker=markers.get(method, 'o'), linestyle='-', 
                 color=colors[idx], linewidth=2, markersize=8,
                 label=lbl)
                 
    plt.title('Execution Time vs Array Size', fontsize=16)
    plt.xlabel('Array Size (Elements)', fontsize=14)
    plt.ylabel('Execution Time (ms)', fontsize=14)
    
    sizes = sorted(df['Elements'].unique())
    plt.xticks(sizes, [f"{int(s//1000)}k" if s < 1000000 else f"{int(s//1000000)}M" for s in sizes])
    
    plt.legend(fontsize=12)
    plt.tight_layout()
    plt.savefig('time_plot.png', dpi=300)
    print("Plot successfully saved to time_plot.png")

if __name__ == "__main__":
    main()
