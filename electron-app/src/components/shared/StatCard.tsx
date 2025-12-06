interface StatCardProps {
  title: string;
  value: string;
  subtitle: string;
  gradient: string;
  icon: string;
}

const StatCard = ({ title, value, subtitle, gradient, icon }: StatCardProps) => {
  return (
    <div className="glass rounded-2xl p-6 shadow-xl hover:shadow-2xl transform hover:scale-[1.02] transition-all duration-300 animate-fade-in-up">
      <div className="flex items-start justify-between mb-4">
        <div>
          <p className="text-sm text-gray-600 font-medium">{title}</p>
          <h3 className="text-3xl font-bold text-gray-900 mt-1">{value}</h3>
        </div>
        <div
          className={`w-12 h-12 rounded-xl bg-gradient-to-br ${gradient} flex items-center justify-center text-2xl shadow-lg`}
        >
          {icon}
        </div>
      </div>
      <p className="text-xs text-gray-500">{subtitle}</p>
    </div>
  );
};

export default StatCard;
